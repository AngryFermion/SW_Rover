/* Copyright 2017-2024 The MathWorks, Inc. */

#ifndef MATLAB_EXECUTION_INTERFACE_IMPL_HPP
#define MATLAB_EXECUTION_INTERFACE_IMPL_HPP

#include <string>
#include <typeinfo>
#include <vector>
#include <streambuf>
#include <memory>
#include <future>
#include <complex>
#include <MatlabEngine/cpp_engine_api.hpp>
#include <MatlabExecutionInterface/execution_interface.hpp>
#include <MatlabExecutionInterface/detail/value_future_impl.hpp>
#include <MatlabExecutionInterface/exception.hpp>
#include <MatlabExecutionInterface/task_reference.hpp>
#include <MatlabDataArray/detail/HelperFunctions.hpp>
#include <MatlabDataArray/StructArray.hpp>
#include <MatlabDataArray/StructRef.hpp>
#include <MatlabDataArray/Struct.hpp>
#include <MatlabDataArray/CharArray.hpp>
#include <MatlabDataArray/TypedArray.hpp>
#include <MatlabDataArray/Reference.hpp>


//Type printing utilities
#ifdef __GNUC__
#include <cxxabi.h>
#include <cstdlib>
#include <memory>

// Demangle function for GCC and Clang
namespace {
    std::string demangle(const char* name)
    {
        int status = -1;
        std::unique_ptr<char, void(*)(void*)> result(
            abi::__cxa_demangle(name, nullptr, nullptr, &status),
            std::free
        );
        return (status == 0) ? result.get() : name;
    }
}

#elif defined(_MSC_VER)
#include <sstream>
#include <vector>

// Demangle function for visual studio compilers
namespace {
    std::string demangle(const char* name)
    {
        std::vector<std::string> result;
        std::stringstream ss(name);
        std::string generalType, specificType;

        getline(ss, generalType, ' '); // get first word before space

        getline(ss, specificType); // rest of output should be the specific type

        return specificType;
    }
}

#else

// Dummy demangle function for other compilers
namespace {
    std::string demangle(const char* name)
    {
        return name;
    }
}

#endif

// Type printing utilities
namespace {
    // Pass by argument
    template<typename T>
    std::string print_type(T)
    {
        using U1 = typename std::remove_reference<T>::type; // strip forwarding reference
        using U2 = typename std::remove_cv<U1>::type;       // remove const/volatile-ness of the type
        using U3 = typename std::remove_pointer<U2>::type;  // remove simple pointer types
        using U4 = typename std::remove_cv<U3>::type;       // remove const/volatile-ness of type the simple pointer is pointing to
        return ::demangle(typeid(U4).name());
    }

    // Pass type explicitly
    template<typename T>
    std::string print_type()
    {
        using U1 = typename std::remove_reference<T>::type; // strip forwarding reference
        using U2 = typename std::remove_cv<U1>::type;       // remove const/volatile-ness of the type
        using U3 = typename std::remove_pointer<U2>::type;  // remove simple pointer types
        using U4 = typename std::remove_cv<U3>::type;       // remove const/volatile-ness of type the simple pointer is pointing to
        return ::demangle(typeid(U4).name());
    }

}

// Base templates; this is empty because in the default case, we have no
// library association for a type
template<typename T>
struct LibraryAssociationOutput { // for output types
};

template<typename T>
struct LibraryAssociationInput { // for input types
};

// Defines a specialization of LibraryAssociation which holds the given
// library name; we'll later leverage the presence of the name to implement
// a predicate which can be used to identify types which have been
// set up using this macro. Multiple specializations clearly denote
// which input type variations and which output type variations are supported.

#define USING_TYPE_WITH_MATLAB(T, Name) \
template<> \
struct LibraryAssociationOutput<T*> { \
    static constexpr char const* value = Name;\
};\
template<> \
struct LibraryAssociationInput<T*> { \
    static constexpr char const* value = Name;\
};\
template<> \
struct LibraryAssociationInput<const T*> { \
    static constexpr char const* value = Name;\
};\
template<> \
struct LibraryAssociationInput<T* const> { \
    static constexpr char const* value = Name;\
};\
template<> \
struct LibraryAssociationInput<const T* const> { \
    static constexpr char const* value = Name;\
};

// Predicate or type trait for identifying types that have been passed to
// USING_TYPE_WITH_MATLAB; this is the base template
template<typename T, typename = void>
struct hasLibraryAssociationOutput : std::false_type {};

// Partial specialization which picks up types which have been set up
// using USING_TYPE_WITH_MATLAB by looking for the embedded "value"
template<typename T>
struct hasLibraryAssociationOutput<
	T,
	typename std::enable_if<LibraryAssociationOutput<typename std::remove_reference<T>::type>::value != nullptr>::type
> : std::true_type {};

// Predicate or type trait for identifying types that have been passed to
// USING_TYPE_WITH_MATLAB; this is the base template
template<typename T, typename = void>
struct hasLibraryAssociationInput : std::false_type {};

// Partial specialization which picks up types which have been set up
// using USING_TYPE_WITH_MATLAB by looking for the embedded "value"
template<typename T>
struct hasLibraryAssociationInput<
	T,
	typename std::enable_if<LibraryAssociationInput<typename std::remove_reference<T>::type>::value != nullptr>::type
> : std::true_type {};


// Returns the name associated with type T as established by
// USING_TYPE_WITH_MATLAB
template<typename T>
constexpr typename std::enable_if<hasLibraryAssociationOutput<T>::value == true, char const*>::type
getLibraryNameOutput()
{
	using nonRefType = typename std::remove_reference<T>::type;
	return LibraryAssociationOutput<nonRefType>::value;
}

// Returns empty string if there is no associaiton
template<typename T>
constexpr typename std::enable_if<hasLibraryAssociationOutput<T>::value == false, char const*>::type
getLibraryNameOutput()
{
	return "";
}

template<typename T>
constexpr typename std::enable_if<hasLibraryAssociationInput<T>::value == true, char const*>::type
getLibraryNameInput()
{
	using nonRefType = typename std::remove_reference<T>::type;
	return LibraryAssociationInput<nonRefType>::value;
}

// Returns empty string if there is no associaiton
template<typename T>
constexpr typename std::enable_if<hasLibraryAssociationInput<T>::value == false, char const*>::type
getLibraryNameInput()
{
	return "";
}

// Utilities for checking output types for custom types with C++ interface library associations

namespace {

    // Holds info for an output with custom type
    struct CustomOutputInfo {
        uint64_t OutputPosition = 0;
        std::string CPPName = "unset";
        std::string LibraryName = "unset";
    };

    // Helper template to check if a type is a tuple
    template<typename T>
    struct is_tuple {
        const static bool value = false;
    };

    template<typename... Args>
    struct is_tuple<std::tuple<Args...>> {
        static const bool value = true;
    };

    // Implementation for recording custom output data of tuple
    template<typename Tuple, std::size_t tupleIndex>
    void recordInfoImpl(std::vector<CustomOutputInfo>& customOutputs) {
        bool isOutputCustom = hasLibraryAssociationOutput<typename std::tuple_element<tupleIndex, Tuple>::type>();
        if(isOutputCustom) {
            CustomOutputInfo outArg;
            outArg.OutputPosition = tupleIndex + 1; // +1 for C++ -> MATLAB indexing
            outArg.CPPName = print_type<typename std::tuple_element<tupleIndex, Tuple>::type>();
            outArg.LibraryName = getLibraryNameOutput<typename std::tuple_element<tupleIndex, Tuple>::type>();
            customOutputs.push_back(outArg);
        }
    }

    // Forward declaration for template to iterate over tuple elements
    template<typename Tuple, std::size_t tupleIndex = 0, bool tupleEnd = (tupleIndex >= std::tuple_size<Tuple>::value - 1)>
    struct TupleInfoHelper;

    // The main template specialization to iterate over tuple elements
    template<typename Tuple, std::size_t tupleIndex>
    struct TupleInfoHelper<Tuple, tupleIndex, false> {
        static void recordInfo(std::vector<CustomOutputInfo>& customOutputs) {
            recordInfoImpl<Tuple, tupleIndex>(customOutputs); // record tuple element info if applicable
            TupleInfoHelper<Tuple, tupleIndex+1>::recordInfo(customOutputs); // recurse to next tuple element
        }
    };

    // Specialization for when the last tuple element is reached
    template<typename Tuple, std::size_t tupleIndex>
    struct TupleInfoHelper<Tuple, tupleIndex, true> {
        static void recordInfo(std::vector<CustomOutputInfo>& customOutputs) {
            recordInfoImpl<Tuple, tupleIndex>(customOutputs); // record tuple element info if applicable
            // no recursion since this is the last element
        }
    };

    // Fuction to create the vector for void type
    template<typename T>
    typename std::enable_if<std::is_void<T>::value, std::vector<CustomOutputInfo>>::type
    getCustomOutputInfo() {
        // Return an empty vector, since void means no outputs
        return std::vector<CustomOutputInfo>{};
    }

    // Function to create the vector and initiate the checking process for tuples (multiple outputs)
    template<typename T>
    typename std::enable_if<is_tuple<T>::value, std::vector<CustomOutputInfo>>::type
    getCustomOutputInfo() {
        std::vector<CustomOutputInfo> customOutputs = {};
        TupleInfoHelper<T>::recordInfo(customOutputs);  // populate customOutputs
        return customOutputs;
    }

    // Function to create the vector for non-tuples (one output)
    template<typename T>
    typename std::enable_if<!is_tuple<T>::value && !std::is_void<T>::value, std::vector<CustomOutputInfo>>::type
    getCustomOutputInfo() {
        std::vector<CustomOutputInfo> customOutputs = {};
        size_t k = 0;
        bool isOutputCustom = hasLibraryAssociationOutput<T>();
        CustomOutputInfo outArg;
        if(isOutputCustom) {
            outArg = CustomOutputInfo();
            outArg.OutputPosition = k + 1; // +1 for C++ -> MATLAB indexing
            outArg.CPPName = print_type<T>();
            outArg.LibraryName = getLibraryNameOutput<T>();
            customOutputs.push_back(outArg);
        }

        return customOutputs;
    }
}

namespace {

    inline matlab::execution::MATLABExecutionException createMATLABExecutionException(const matlab::data::StructArray& mException);

    inline std::vector<matlab::execution::MATLABExecutionException> createCause(const matlab::data::CellArray& cause) {
        size_t nCauses = cause.getNumberOfElements();
        std::vector<matlab::execution::MATLABExecutionException> causes(nCauses);
        for (size_t i = 0; i < nCauses; i++) {
            matlab::data::Array exRef = cause[i];
            matlab::data::StructArray ex(exRef);
            causes[i] = createMATLABExecutionException(ex);
        }
        return causes;
    }

    inline std::vector<matlab::execution::StackFrame> createStackTrace(const matlab::data::StructArray& stack) {
        size_t nFrames = stack.getNumberOfElements();
        std::vector<matlab::execution::StackFrame> stackFrames(nFrames);

        for (size_t i = 0; i < nFrames; i++) {
            matlab::data::Array fileRef = stack[i]["File"];
            matlab::data::CharArray fileStr(fileRef);

            matlab::data::Array nameRef = stack[i]["Name"];
            matlab::data::CharArray nameStr(nameRef);

            matlab::data::Array lineRef = stack[i]["Line"];
            double line = lineRef[0];
            stackFrames[i] = matlab::execution::StackFrame(fileStr.toUTF16(), nameStr.toUTF16(), uint32_t(line));
        }

        return stackFrames;
    }

    inline matlab::execution::MATLABExecutionException createMATLABExecutionException(const matlab::data::StructArray& mException) {
        matlab::data::Array idRef = mException[0][std::string("identifier")];
        matlab::data::CharArray id(idRef);
        matlab::data::Array messageRef = mException[0][std::string("message")];
        matlab::data::CharArray message(messageRef);

        matlab::data::Array stackRef = mException[0][std::string("stack")];
        matlab::data::StructArray stack(stackRef);

        matlab::data::Array causeRef = mException[0][std::string("cause")];
        matlab::data::CellArray cause(causeRef);

        std::vector<matlab::execution::MATLABExecutionException> meCause = createCause(cause);
        std::vector<matlab::execution::StackFrame> meStack = createStackTrace(stack);
        return matlab::execution::MATLABExecutionException(id.toAscii(), message.toUTF16(), meStack, meCause);
    }

    inline matlab::execution::MATLABSyntaxException createMATLABSyntaxException(const matlab::data::StructArray& mException) {
        matlab::data::Array idRef = mException[0][std::string("identifier")];
        matlab::data::CharArray id(idRef);
        matlab::data::Array messageRef = mException[0][std::string("message")];
        matlab::data::CharArray message(messageRef);

        return matlab::execution::MATLABSyntaxException(id.toAscii(), message.toUTF16());
    }

    template<typename T>
    inline void set_promise_exception(void *p, size_t excTypeNumber, const void* msg) {
        std::promise<T>* prom = reinterpret_cast<std::promise<T>*>(p);
        ::detail::ExceptionType excType = static_cast<::detail::ExceptionType>(excTypeNumber);
        switch (excType) {
        case ::detail::ExceptionType::CANCELLED:{
            const char* message = reinterpret_cast<const char*>(msg);
            prom->set_exception(std::make_exception_ptr<matlab::execution::CancelledException>(matlab::execution::CancelledException(message)));
            break;
        }
        case ::detail::ExceptionType::INTERRUPTED: {
            const char* message = reinterpret_cast<const char*>(msg);
            prom->set_exception(std::make_exception_ptr<matlab::execution::InterruptedException>(matlab::execution::InterruptedException(message)));
            break;
        }
        case ::detail::ExceptionType::EXECUTION:
        case ::detail::ExceptionType::SYNTAX: {
            matlab::data::impl::ArrayImpl* exceptionImpl = const_cast<matlab::data::impl::ArrayImpl*>(reinterpret_cast<const matlab::data::impl::ArrayImpl*>(msg));
            matlab::data::Array mdaException = matlab::data::detail::Access::createObj<matlab::data::Array>(exceptionImpl);
            matlab::data::StructArray mException(mdaException);
            if (excType == ::detail::ExceptionType::SYNTAX) {
                prom->set_exception(std::make_exception_ptr<matlab::execution::MATLABSyntaxException>(createMATLABSyntaxException(mException)));
            }
            else {
                prom->set_exception(std::make_exception_ptr<matlab::execution::MATLABExecutionException>(createMATLABExecutionException(mException)));
            }
            break;
        }
        case ::detail::ExceptionType::OTHER: {
            const char* message = reinterpret_cast<const char*>(msg);
            prom->set_exception(std::make_exception_ptr<matlab::execution::Exception>(matlab::execution::Exception(message)));
            break;
        }
        case ::detail::ExceptionType::STOPPED: {
            const char* message = reinterpret_cast<const char*>(msg);
            prom->set_exception(std::make_exception_ptr<matlab::execution::MATLABNotAvailableException>(matlab::execution::MATLABNotAvailableException(message)));
            break;
        }
        break;
        }
        delete prom;
    }
}

namespace matlab {
    namespace execution {

        inline ExecutionInterface::ExecutionInterface(uint64_t handle) : matlabHandle(handle) {
        }

        inline void set_eval_promise_data(void *p) {
            std::promise<void>* prom = reinterpret_cast<std::promise<void>*>(p);
            prom->set_value();
            delete prom;
        }

        inline void set_eval_promise_exception(void *p, size_t excTypeNumber, const void* msg) {
            set_promise_exception<void>(p, excTypeNumber, msg);
        }

        inline void set_feval_promise_data(void *p, size_t nlhs, bool straight, matlab::data::impl::ArrayImpl** plhs) {

            if (nlhs == 0 && straight) {
                std::promise<void>* prom = reinterpret_cast<std::promise<void>*>(p);
                prom->set_value();
                delete prom;
                return;
            }

            if (nlhs == 1 && straight) {
                std::promise<matlab::data::Array>* prom = reinterpret_cast<std::promise<matlab::data::Array>*>(p);
                matlab::data::Array v_ = matlab::data::detail::Access::createObj<matlab::data::Array>(plhs[0]);
                prom->set_value(v_);
                delete prom;
                return;
            }

            std::promise<std::vector<matlab::data::Array> >* prom = reinterpret_cast<std::promise<std::vector<matlab::data::Array> >*>(p);
            std::vector<matlab::data::Array> result;
            for (size_t i = 0; i < nlhs; i++) {
                matlab::data::Array v_ = matlab::data::detail::Access::createObj<matlab::data::Array>(plhs[i]);
                result.push_back(v_);
            }
            prom->set_value(result);
            delete prom;
        }

        template<class T>
        void set_exception(T p, std::exception_ptr e) {
            p->set_exception(e);
        }

        inline void set_feval_promise_exception(void *p, size_t nlhs, bool straight, size_t excTypeNumber, const void* msg) {
            if (nlhs == 0 && straight) {
                set_promise_exception<void>(p, excTypeNumber, msg);
            }
            else if (nlhs == 1 && straight) {
                set_promise_exception<matlab::data::Array>(p, excTypeNumber, msg);
            }
            else {
                set_promise_exception<std::vector<matlab::data::Array>>(p, excTypeNumber, msg);
            }
        }

        inline std::vector<matlab::data::Array> ExecutionInterface::feval(const std::u16string &function,
                                                             const size_t nlhs,
                                                             const std::vector<matlab::data::Array> &args,
                                                             const std::shared_ptr<StreamBuffer>& output,
                                                             const std::shared_ptr<StreamBuffer>& error) {
            return fevalAsync(function, nlhs, args, output, error).get();
        }

        inline std::vector<matlab::data::Array> ExecutionInterface::feval(const std::string &function,
                                                             const size_t nlhs,
                                                             const std::vector<matlab::data::Array> &args,
                                                             const std::shared_ptr<StreamBuffer>& output,
                                                             const std::shared_ptr<StreamBuffer>& error) {
            return feval(std::u16string(function.cbegin(), function.cend()), nlhs, args, output, error);
        }


        inline matlab::data::Array ExecutionInterface::feval(const std::u16string &function,
            const std::vector<matlab::data::Array> &args,
            const std::shared_ptr<StreamBuffer>& output,
            const std::shared_ptr<StreamBuffer>& error) {
            FutureResult<matlab::data::Array> future = fevalAsync(function, args, output, error);
            return future.get();
        }

        inline matlab::data::Array ExecutionInterface::feval(const std::string &function,
            const std::vector<matlab::data::Array> &args,
            const std::shared_ptr<StreamBuffer>& output,
            const std::shared_ptr<StreamBuffer>& error) {
            return feval(std::u16string(function.cbegin(), function.cend()), args, output, error);
        }

        inline matlab::data::Array ExecutionInterface::feval(const std::u16string &function,
        const matlab::data::Array &arg,
        const std::shared_ptr<StreamBuffer>& output,
        const std::shared_ptr<StreamBuffer>& error) {
            FutureResult<matlab::data::Array> future = fevalAsync(function, arg, output, error);
            return future.get();
        }

        inline matlab::data::Array ExecutionInterface::feval(const std::string &function,
        const matlab::data::Array &arg,
        const std::shared_ptr<StreamBuffer>& output,
        const std::shared_ptr<StreamBuffer>& error) {
            return feval(std::u16string(function.cbegin(), function.cend()), arg, output, error);
        }

        template<class ReturnType, typename...RhsArgs>
        ReturnType ExecutionInterface::feval(const std::u16string &function,
        const std::shared_ptr<StreamBuffer>& output,
        const std::shared_ptr<StreamBuffer>& error,
        RhsArgs&&... rhsArgs
        ) {
            return fevalAsync<ReturnType>(function, output, error, std::forward<RhsArgs>(rhsArgs)...).get();
        }

        template<class ReturnType, typename...RhsArgs>
        ReturnType ExecutionInterface::feval(const std::string &function,
        const std::shared_ptr<StreamBuffer>& output,
        const std::shared_ptr<StreamBuffer>& error,
        RhsArgs&&... rhsArgs
        ) {
            return feval<ReturnType>(std::u16string(function.cbegin(), function.cend()),
                output, error, std::forward<RhsArgs>(rhsArgs)...);
        }

        namespace detail {
            template <typename T>
            struct is_complex_t : public std::false_type {};
            template <typename T>
            struct is_complex_t<std::complex<T>> : public std::true_type {};

            template <typename T>
            struct is_vector_t : public std::false_type {};
            template <typename T, typename A>
            struct is_vector_t<std::vector<T, A>> : public std::true_type {};

            // Returns true if type is a supported, simple C++ primitive type or a supported type that's included in standard libraries
            template<typename T>
            inline constexpr bool isSupportedStandardCPPType() {
                using U = typename std::remove_cv<typename std::remove_reference<T>::type>::type; // strip any const, volatile, and reference qualifiers
                return (std::is_same<U, bool>::value
                    || std::is_same<U, int>::value
                    || std::is_same<U, int8_t>::value
                    || std::is_same<U, int16_t>::value
                    || std::is_same<U, int32_t>::value
                    || std::is_same<U, int64_t>::value
                    || std::is_same<U, uint8_t>::value
                    || std::is_same<U, uint16_t>::value
                    || std::is_same<U, uint32_t>::value
                    || std::is_same<U, uint64_t>::value
                    || std::is_same<U, char>::value
                    || std::is_convertible<U, std::basic_string<char>>()
                    || std::is_convertible<U, std::basic_string<char16_t>>()
                    || std::is_same<U, float>::value
                    || std::is_same<U, double>::value
                    || is_complex_t<U>()
                    || is_vector_t<U>());
            }

            // Returns true if type is a supported user-defined custom type that should work with a CPP interface library
            template<typename T>
            inline constexpr bool isSupportedCustomTypeOutput() {
                return hasLibraryAssociationOutput<T>(); // Check if type has a declared CPP interface library association
            }

            // Returns true if type is a supported user-defined custom type that should work with a CPP interface library
            template<typename T>
            inline constexpr bool isSupportedCustomTypeInput() {
                return hasLibraryAssociationInput<T>(); // Check if input type has a declared CPP interface library association
            }

            // Returns true if T is a supported output type
            template<typename T>
            inline constexpr bool isSupportedOutput() {
                return isSupportedStandardCPPType<T>() || isSupportedCustomTypeOutput<T>(); // check if it's a suported standard type or a supported custom output type
            }

            // Returns true if T is a supported input type
            template<typename T>
            inline constexpr bool isSupportedInput() {
                return isSupportedStandardCPPType<T>() || isSupportedCustomTypeInput<T>(); // check if it's a suported standard type or a supported custom input type
            }

            // Compile-time check which errors if input type is not supported, returns true otherwise
            template<typename T>
            inline constexpr bool validateTIsSupportedInput() {
                static_assert(isSupportedInput<T>()
                    , "Attempted to use unsupported types.");
                return isSupportedInput<T>(); // return type of constexpr can't be void in C++11
            }

            // Compile-time check which errors if output type is not supported, returns true otherwise
            template<typename T>
            inline constexpr bool validateTIsSupportedOutput() {
                static_assert(isSupportedOutput<T>()
                    , "Attempted to use unsupported types.");
                return isSupportedOutput<T>(); // return type of constexpr can't be void in C++11
            }

            inline matlab::data::Array createArray(matlab::data::ArrayFactory& factory, std::basic_string<char16_t>&& value)
            {
                validateTIsSupportedInput<std::basic_string<char16_t>>();
                std::u16string new_valueInput(value.cbegin(),value.cend());
                return factory.createScalar(new_valueInput);
            }

            template<size_t N>
            matlab::data::Array createArray(matlab::data::ArrayFactory& factory, char const (&str)[N])
            {
                validateTIsSupportedInput<char>();
                std::string new_valueInput(str);
                return factory.createScalar(new_valueInput);
            }


            template<typename T>
            matlab::data::Array createArray(matlab::data::ArrayFactory& factory, T&& value)
            {
                validateTIsSupportedInput<T>();
                using U = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
                return factory.createArray<U>({ 1, 1 }, { value });
            }

            template <typename T,
                typename std::enable_if<!isSupportedInput<T>()>::type* = nullptr> // Overload is only enabled if type is not supported
            matlab::data::Array createRhs(matlab::data::ArrayFactory& factory, T&& value) {

                static_assert(isSupportedInput<T>(), "Attempted to use unsupported types."); // expected to fail compilation if this is the overload available
                matlab::data::Array junk; // should be unreachable
                return junk; // having return of matlab::data::Array is technically needed for compilers
            }

            template <class T,
                typename std::enable_if<isSupportedStandardCPPType<T>()>::type* = nullptr> // Overload is only enabled if type is a standard supported type
            matlab::data::Array createRhs(matlab::data::ArrayFactory& factory, T&& value) {

                using NonRefType = typename std::remove_reference<T>::type;
                return createArray(factory, value);  // general case where we have a more simple type
            }

            template <typename T,
                typename std::enable_if<isSupportedCustomTypeInput<T>()>::type* = nullptr>  // Overload is only enabled if type is a supported user-defined type
            matlab::data::Array createRhs(matlab::data::ArrayFactory& factory, T&& value) {
                // Case where we have a custom type

                // Create the wrapper MATLAB struct which will hold data about the user-defined data
                matlab::data::StructArray wrapper = factory.createStructArray({1,1}, { "MATLABCompatibleStruct", "Address", "CPPName", "LibraryName", "IsDataConst"});

                // get library name
                using NonRefType = typename std::remove_reference<T>::type; // remove reference from std::forward
                std::string libName = getLibraryNameInput<NonRefType>();

                // get struct type as a string in C++
                std::string structType = ::print_type(value);

                // determine if the data is const
                using U1 = typename std::remove_reference<T>::type; // strip forwarding reference
                using U2 = typename std::remove_cv<U1>::type;       // remove const/volatile-ness of the type
                using U3 = typename std::remove_pointer<U2>::type;  // remove simple pointer types
                using U4 = typename std::remove_cv<U3>::type;       // remove const/volatile-ness of type the simple pointer is pointing to

                uint64_t address; // address to the data
                bool isconst;  // if the data is constant

                address = reinterpret_cast<uint64_t>(value); // address is the pointer value itself
                isconst = std::is_const<U3>::value; // const-ness of data that pointer refers to

                // Populate the wrapper struct
                wrapper[0]["MATLABCompatibleStruct"] = factory.createScalar<bool>(true);
                wrapper[0]["Address"] = factory.createScalar<uint64_t>(address);
                wrapper[0]["CPPName"] = factory.createCharArray(structType);
                wrapper[0]["LibraryName"] = factory.createCharArray(libName);
                wrapper[0]["IsDataConst"] = factory.createScalar<bool>(isconst);

                // return the metadata of the input arg. We can check for special fields like "MATLABCompatibleStruct" to differentiate from regular inputs.
                return wrapper;
            }

            template <typename T, typename A>
            matlab::data::Array createRhs(matlab::data::ArrayFactory& factory,
                                          std::vector<T, A>& value) {
                validateTIsSupportedInput<T>();
                return factory.createArray({1, value.size()}, value.begin(), value.end());
            }

            template<typename T, typename A>
            matlab::data::Array createRhs(matlab::data::ArrayFactory& factory, std::vector <T, A>&& value) {
                validateTIsSupportedInput<T>();
                return factory.createArray({ 1, value.size() }, value.begin(), value.end());
            }

            inline matlab::data::Array createRhs(matlab::data::ArrayFactory& factory, matlab::data::Array& value) {
                (void) factory;
                return value;
            }

            template <std::size_t ...Ints>
            struct index_sequence {
                using value_type = std::size_t;
                static std::size_t size() { return sizeof...(Ints); }
            };

            template<std::size_t N, std::size_t... Values>
            struct make_index_sequence_impl {
                using type = typename make_index_sequence_impl<N - 1, Values..., sizeof...(Values)>::type;
            };

            template<std::size_t... Values>
            struct make_index_sequence_impl < 0, Values... > {
                using type = index_sequence < Values... > ;
            };

            template<std::size_t N>
            using make_index_sequence = typename make_index_sequence_impl<N>::type;

            template<typename T> // case where we have 1 output (not void or tuple)
            struct createLhs {
                static const size_t nlhs = 1;

                template <typename T2 = T, // use T2 here to make sure subtitution failure occurs if needed
                    typename std::enable_if<!isSupportedOutput<T2>()>::type* = nullptr> // Overload is only enabled if type is not supported
                T2 operator()(std::vector<matlab::data::Array>&& lhs) const { // case where we have 1 user-defined output in std::vector<matlab::data::Array> with length=1
                    static_assert(isSupportedOutput<T2>(), "Attempted to use unsupported types."); // expected to fail compilation if this is the overload available
                    T2 junk;
                    return junk; // Satisfies return type requirement of C++11 compilation. Should be unreachable.
                }

                template <typename T2 = T, // use T2 here to make sure subtitution failure occurs if needed
                    typename std::enable_if<isSupportedCustomTypeOutput<T2>()>::type* = nullptr> // Overload is only enabled if type is a supported user-defined type
                T2 operator()(std::vector<matlab::data::Array>&& lhs) const { // case where we have 1 user-defined output in std::vector<matlab::data::Array> with length=1

                    if (lhs.empty()) {
                        throw matlab::execution::TypeConversionException("The result is empty.");
                    }

                    T2 value;
                    try {
                        // if type is a special struct pointer we know the MDA contains a 64-bit address
                        uint64_t address = ((matlab::data::TypedArray<uint64_t>)lhs.front())[0];
                        value = reinterpret_cast<T2>(address);
                    }
                    catch (const std::exception& e) {
                        throw matlab::execution::TypeConversionException(e.what());
                    }

                    return value;
                }

               template <typename T2 = T, // use T2 here to make sure subtitution failure occurs if needed
                    typename std::enable_if<isSupportedStandardCPPType<T2>()>::type* = nullptr>  // Overload is only enabled if type is a standard supported type
                T2 operator()(std::vector<matlab::data::Array>&& lhs) const { // case where we have 1 standard type output in std::vector<matlab::data::Array> with length=1

                    if (lhs.empty()) {
                        throw matlab::execution::TypeConversionException("The result is empty.");
                    }

                    T2 value;
                    try {
                        value = (*this)(matlab::data::TypedArray<T2>(std::move(lhs.front())));
                    }
                    catch (const std::exception& e) {
                        throw matlab::execution::TypeConversionException(e.what());
                    }
                    return value;
                }

                template <typename T2 = T, // use T2 here to make sure subtitution failure occurs if needed
                    typename std::enable_if<!isSupportedOutput<T2>()>::type* = nullptr> // Overload is only enabled if type is not supported
                T2 operator()(matlab::data::Array lhs) const {  // case where we have 1 user-defined output in a matlab::data::Array

                    static_assert(isSupportedOutput<T2>(), "Attempted to use unsupported types."); // expected to fail compilation if this is the overload available
                    T2 junk;
                    return junk; // Satisfies return type requirement of C++11 compilation. Should be unreachable.
                }

                template <typename T2 = T, // use T2 here to make sure subtitution failure occurs if needed
                    typename std::enable_if<isSupportedStandardCPPType<T2>()>::type* = nullptr>  // Overload is only enabled if type is a standard supported type
                T operator()(matlab::data::TypedArray<T> lhs) const { // case where we have 1 standard type output in a TypedArray
                    validateTIsSupportedOutput<T>();
                    auto const begin = lhs.begin();
                    auto const end = lhs.end();
                    if (begin == end) {
                        throw matlab::execution::TypeConversionException("The result is empty.");
                    }
                    return *begin;
                }

                template <typename T2 = T, // use T2 here to make sure subtitution failure occurs if needed
                    typename std::enable_if<isSupportedCustomTypeOutput<T2>()>::type* = nullptr>  // Overload is only enabled if type is a supported user-defined type
                T2 operator()(matlab::data::Array lhs) const {  // case where we have 1 user-defined output in a matlab::data::Array

                    if (lhs.isEmpty()) {
                        throw matlab::execution::TypeConversionException("The result is empty.");
                    }

                    T2 value;
                    try {
                        // if type is a special struct pointer we know the MDA contains a 64-bit address
                        uint64_t address = ((matlab::data::TypedArray<uint64_t>)lhs)[0];
                        value = reinterpret_cast<T2>(address);
                    }
                    catch (const std::exception& e) {

                        throw matlab::execution::TypeConversionException(e.what());
                    }
                    return value;
                }
            };

            template<>
            struct createLhs < void > {
                static const size_t nlhs = 0;
                void operator()(std::vector<matlab::data::Array>&& lhs) const {
                    (void) lhs;
                }
            };

            template<>
            struct createLhs<std::vector<std::string>> {
                static const size_t nlhs = 1;

                std::vector<std::string> operator()(std::vector<matlab::data::Array>&& lhs) const {
                    if (lhs.empty()) {
                        throw matlab::execution::TypeConversionException("The result is empty.");
                    }
                    std::vector<std::string> resultVector;
                    try {
                        for( auto matlabArray : lhs )
                        {
                            if (matlabArray.getType() == matlab::data::ArrayType::MATLAB_STRING)
                            {
                                matlab::data::TypedArray<matlab::data::MATLABString> matlabString = matlabArray;
                                for( auto r : matlabString)
                                {
                                    std::string output = r;
                                    resultVector.push_back(output);
                                }
                            }
                            else if (matlabArray.getType() == matlab::data::ArrayType::CHAR)
                            {
                                matlab::data::CharArray charArray = matlabArray;
                                std::string output = charArray.toAscii();
                                resultVector.push_back(output);
                            }
                            else
                            {
                                throw matlab::data::InvalidArrayTypeException("The return value is not a string (or) char.");
                            }
                        }
                    } catch (const std::exception& e) {
                        throw matlab::execution::TypeConversionException(e.what());
                    }

                    return resultVector;
                }
            };

            template<>
            struct createLhs<std::vector<std::u16string>> {
                static const size_t nlhs = 1;

                std::vector<std::u16string> operator()(std::vector<matlab::data::Array>&& lhs) const {
                    if (lhs.empty()) {
                        throw matlab::execution::TypeConversionException("The result is empty.");
                    }

                    std::vector<std::u16string> resultVector;
                    try {
                        for( auto matlabArray : lhs )
                        {
                            if (matlabArray.getType() == matlab::data::ArrayType::MATLAB_STRING)
                            {
                                matlab::data::TypedArray<matlab::data::MATLABString> matlabString = matlabArray;
                                for( auto r : matlabString)
                                {
                                    std::u16string output = r;
                                    resultVector.push_back(output);
                                }
                            }
                            else if (matlabArray.getType() == matlab::data::ArrayType::CHAR)
                            {
                                matlab::data::CharArray valueCArray = matlabArray;
                                std::u16string output = valueCArray.toUTF16();
                                resultVector.push_back(output);
                            }
                            else
                            {
                                throw matlab::data::InvalidArrayTypeException("The return value is not a string (or) char.");
                            }
                        }
                    } catch (const std::exception& e) {
                        throw matlab::execution::TypeConversionException(e.what());
                    }

                    return resultVector;
                }
            };

            template<typename T, typename A>
            struct createLhs <std::vector<T, A>> {
                static const size_t nlhs = 1;

                std::vector<T, A> operator()(std::vector<matlab::data::Array>&& lhs) const {
                    if (lhs.empty()) {
                        throw matlab::execution::TypeConversionException("The result is empty.");
                    }

                    std::vector<T, A> resultVector;
                    try {
                        matlab::data::TypedArray<T> value(std::move(lhs.front()));
                        for (auto elem : value) {
                            resultVector.push_back(elem);
                        }
                    } catch (const std::exception& e) {
                        throw matlab::execution::TypeConversionException(e.what());
                    }

                    return resultVector;
                }

                std::vector<T,A> operator()(matlab::data::TypedArray<T> lhs) const {
                  auto const begin = lhs.begin();
                  auto const end = lhs.end();
                  if (begin == end) {
                      throw matlab::execution::TypeConversionException("The result is empty.");
                  }

                  std::vector<T, A> resultVector;
                  try {
                      for (auto elem : lhs) {
                          resultVector.push_back(elem);
                      }
                  } catch (const std::exception& e) {
                      throw matlab::execution::TypeConversionException(e.what());
                  }

                  return resultVector;
                }
            };

            template<typename... TupleTypes>
            struct createLhs < std::tuple<TupleTypes...> > {
                static const size_t nlhs = sizeof...(TupleTypes);
                using T = std::tuple < TupleTypes... > ;

                T operator()(std::vector<matlab::data::Array>&& lhs) const {
                    //we are not validating the LHS here as it can be any combinations of types for std::tuple.
                    if (lhs.size() < sizeof...(TupleTypes)) { throw std::runtime_error(""); }
                    return (*this)(std::move(lhs), detail::make_index_sequence<sizeof...(TupleTypes)>());
                }

            private:
                template<size_t Index>
                using TupleElement = typename std::remove_cv<typename std::remove_reference<typename std::tuple_element<Index, std::tuple<TupleTypes...> >::type>::type>::type;

                template<size_t... IndexList>
                std::tuple <TupleTypes...> operator()(std::vector<matlab::data::Array>&& lhs, detail::index_sequence<IndexList...>) const {
                    return std::tuple <TupleTypes...>(createLhs<TupleElement<IndexList>>()(std::move(lhs[IndexList]))...);
                }
            };

           template<>
           struct createLhs<std::string> {
               static const size_t nlhs = 1;

               std::string operator()(std::vector<matlab::data::Array>&& lhs) const {
                   if (lhs.empty()) {
                       throw matlab::execution::TypeConversionException("The result is empty.");
                   }
                   std::string value;
                   try {
                       matlab::data::Array type = lhs.front();
                       if (type.getType() == matlab::data::ArrayType::MATLAB_STRING)
                       {
                           matlab::data::TypedArray<data::MATLABString> valueMString(std::move(lhs.front()));
                           value = valueMString[0];
                       }
                       else if (type.getType() == matlab::data::ArrayType::CHAR)
                       {
                           matlab::data::CharArray valueCArray(std::move(lhs.front()));
                           value = valueCArray.toAscii();
                       }
                       else
                       {
                           throw matlab::data::InvalidArrayTypeException("The return value is not a string (or) char.");
                       }
                   }
                   catch (const std::exception& e) {
                       throw matlab::execution::TypeConversionException(e.what());
                   }
                   return value;
               }

               std::string operator()(matlab::data::TypedArray<data::MATLABString> lhs) const {
                   auto const begin = lhs.begin();
                   auto const end = lhs.end();
                   if (begin == end) {
                       throw matlab::execution::TypeConversionException("The result is empty.");
                   }
                   return *begin;
               }
           };

           template<>
           struct createLhs<std::u16string> {
               static const size_t nlhs = 1;

               std::u16string operator()(std::vector<matlab::data::Array>&& lhs) const {
                   if (lhs.empty()) {
                       throw matlab::execution::TypeConversionException("The result is empty.");
                   }

                   std::basic_string<char16_t> valueBString;
                   std::u16string value;
                   try {
                       matlab::data::Array type = lhs.front();
                       if (type.getType() == matlab::data::ArrayType::MATLAB_STRING)
                       {
                           matlab::data::TypedArray<data::MATLABString> valueMString(std::move(lhs.front()));
                           value = valueMString[0];
                       }
                       else if (type.getType() == matlab::data::ArrayType::CHAR)
                       {
                           matlab::data::CharArray valueCArray(std::move(lhs.front()));
                           value = valueCArray.toUTF16();
                       }
                       else
                       {
                           throw matlab::data::InvalidArrayTypeException("The return value is not a string (or) char.");
                       }
                   }
                   catch (const std::exception& e) {
                       throw matlab::execution::TypeConversionException(e.what());
                   }
                   return value;
               }

               std::u16string operator()(matlab::data::TypedArray<data::MATLABString> lhs) const {
                   auto const begin = lhs.begin();
                   auto const end = lhs.end();
                   if (begin == end) {
                       throw matlab::execution::TypeConversionException("The result is empty.");
                   }
                   return *begin;
               }
           };

           template<>
           struct createLhs<matlab::data::Array> {
               static const size_t nlhs = 1;

               matlab::data::Array operator()(std::vector<matlab::data::Array>&& lhs) const {
                   if (lhs.empty()) {
                       throw matlab::execution::TypeConversionException("The result is empty.");
                   }
                   return lhs[0];
               }

               matlab::data::Array operator()(matlab::data::TypedArray<matlab::data::Array> lhs) const {
                   auto const begin = lhs.begin();
                   auto const end = lhs.end();
                   if (begin == end) {
                       throw matlab::execution::TypeConversionException("The result is empty.");
                   }
                   return *begin;
               }
           };
        }

        template<class ReturnType, typename...RhsArgs>
        ReturnType ExecutionInterface::feval(const std::u16string &function,
                              RhsArgs&&...rhsArgs
        ) {
            const std::shared_ptr<StreamBuffer> defaultStream;
            auto future = fevalAsync<ReturnType>(function, defaultStream, defaultStream, std::forward<RhsArgs>(rhsArgs)...);
            return future.get();
        }

        template<class ReturnType, typename...RhsArgs>
        ReturnType ExecutionInterface::feval(const std::string &function,
                              RhsArgs&&...rhsArgs
        ) {
            return feval<ReturnType>(std::u16string(function.cbegin(), function.cend()),
                std::forward<RhsArgs>(rhsArgs)...);
        }

        inline ExecutionInterface::~ExecutionInterface() {
            matlabHandle = 0;
        }

        inline FutureResult<std::vector<matlab::data::Array> > ExecutionInterface::fevalAsync(const std::u16string &function,
        const size_t nlhs,
        const std::vector<matlab::data::Array> &args,
        const std::shared_ptr<StreamBuffer> &output,
        const std::shared_ptr<StreamBuffer> &error
        ) {

            size_t nrhs = args.size();
            std::unique_ptr<matlab::data::impl::ArrayImpl*, void(*)(matlab::data::impl::ArrayImpl**)> argsImplPtr(new matlab::data::impl::ArrayImpl*[nrhs], [](matlab::data::impl::ArrayImpl** ptr) {
                delete[] ptr;
            });
            matlab::data::impl::ArrayImpl** argsImpl = argsImplPtr.get();
            size_t i = 0;
            for (auto e : args) {
                argsImpl[i++] = matlab::data::detail::Access::getImpl<matlab::data::impl::ArrayImpl>(e);
            }
            std::promise<std::vector<matlab::data::Array> >* p = new std::promise<std::vector<matlab::data::Array> >();
            std::future<std::vector<matlab::data::Array> > f = p->get_future();

            void* output_ = output ? new std::shared_ptr<StreamBuffer>(output) : nullptr;
            void* error_ = error ? new std::shared_ptr<StreamBuffer>(error) : nullptr;
            std::string utf8functionname = convertUTF16StringToASCIIString(function);
            uintptr_t handle = cpp_engine_feval_with_completion(matlabHandle, utf8functionname.c_str(), nlhs, false, argsImpl, nrhs, &set_feval_promise_data, &set_feval_promise_exception, p, output_, error_, &writeToStreamBuffer, &deleteStreamBufferImpl);

            return FutureResult<std::vector<matlab::data::Array>>(std::move(f), std::make_shared<TaskReference>(handle, cpp_engine_cancel_feval_with_completion ));
        }

        inline FutureResult<std::vector<matlab::data::Array> > ExecutionInterface::fevalAsync(const std::string &function,
        const size_t nlhs,
        const std::vector<matlab::data::Array> &args,
        const std::shared_ptr<StreamBuffer> &output,
        const std::shared_ptr<StreamBuffer> &error
        ) {
            return fevalAsync(std::u16string(function.cbegin(), function.cend()), nlhs, args, output, error);
        }

        inline FutureResult<matlab::data::Array> ExecutionInterface::fevalAsync(const std::u16string &function,
        const std::vector<matlab::data::Array> &args,
        const std::shared_ptr<StreamBuffer> &output,
        const std::shared_ptr<StreamBuffer> &error
        ) {
            size_t nrhs = args.size();
            std::unique_ptr<matlab::data::impl::ArrayImpl*, void(*)(matlab::data::impl::ArrayImpl**)> argsImplPtr(new matlab::data::impl::ArrayImpl*[nrhs], [](matlab::data::impl::ArrayImpl** ptr) {
                delete[] ptr;
            });
            matlab::data::impl::ArrayImpl** argsImpl = argsImplPtr.get();
            size_t i = 0;
            for (auto e : args) {
                argsImpl[i++] = matlab::data::detail::Access::getImpl<matlab::data::impl::ArrayImpl>(e);
            }
            std::promise<matlab::data::Array>* p = new std::promise<matlab::data::Array>();
            std::future<matlab::data::Array> f = p->get_future();

            void* output_ = output ? new std::shared_ptr<StreamBuffer>(output) : nullptr;
            void* error_ = error ? new std::shared_ptr<StreamBuffer>(error) : nullptr;

            std::string utf8functionname = convertUTF16StringToASCIIString(function);
            uintptr_t handle = cpp_engine_feval_with_completion(matlabHandle, utf8functionname.c_str(), 1, true, argsImpl, nrhs, &set_feval_promise_data, &set_feval_promise_exception, p, output_, error_, &writeToStreamBuffer, &deleteStreamBufferImpl);

            return FutureResult<matlab::data::Array>(std::move(f), std::make_shared<TaskReference>(handle, cpp_engine_cancel_feval_with_completion));
        }

        inline FutureResult<matlab::data::Array> ExecutionInterface::fevalAsync(const std::string &function,
        const std::vector<matlab::data::Array> &args,
        const std::shared_ptr<StreamBuffer> &output,
        const std::shared_ptr<StreamBuffer> &error
        ) {
            return fevalAsync(std::u16string(function.cbegin(), function.cend()), args, output, error);
        }

        inline FutureResult<matlab::data::Array> ExecutionInterface::fevalAsync(const std::u16string &function,
        const matlab::data::Array &arg,
        const std::shared_ptr<StreamBuffer> &output,
        const std::shared_ptr<StreamBuffer> &error
        ) {
            return fevalAsync(function, std::vector<matlab::data::Array>({ arg }), output, error);
        }

        inline FutureResult<matlab::data::Array> ExecutionInterface::fevalAsync(const std::string &function,
        const matlab::data::Array &arg,
        const std::shared_ptr<StreamBuffer> &output,
        const std::shared_ptr<StreamBuffer> &error
        ) {
            return fevalAsync(std::u16string(function.cbegin(), function.cend()), arg, output, error);
        }

        template<class ReturnType, typename... RhsArgs>
        FutureResult<ReturnType> ExecutionInterface::fevalAsync(const std::u16string &function,
            const std::shared_ptr<StreamBuffer> &output,
            const std::shared_ptr<StreamBuffer> &error,
            RhsArgs&&... rhsArgs
            ) {

            // Transform variadic inputs into vector of matlab::data::Array for input into non-variadic feval
            matlab::data::ArrayFactory factory;
            std::vector<matlab::data::Array> rhsList({
                detail::createRhs(factory, std::forward<RhsArgs>(rhsArgs))...
            });

            // Detect any output types that are custom types that use C++ interface.
            // Will be used to inform feval to release ownwership of C++ interface object and return underlying C++ pointer

            using NonRefType = typename std::remove_reference<ReturnType>::type; // remove reference from std::forward
            std::vector<::CustomOutputInfo> lhsCustomTypes = ::getCustomOutputInfo<NonRefType>();

            std::size_t numCustomOutputs = lhsCustomTypes.size();

            // Fields that will hold lhs output info
            std::vector<std::string> fieldNames =
                { "OutputPosition", // 1 = first output, 2 = second output, etc.
                "CPPName",          // fully qualified C++ name
                "LibraryName"};     // Name of C++ interface lib

            matlab::data::StructArray lhsCustomTypesMDA = factory.createStructArray({1, 0}, fieldNames); // Will hold info on outputs with custom types, empty for now

            if(numCustomOutputs > 0) {

                // Create the wrapper MATLAB struct which will hold data about the user-defined data
                lhsCustomTypesMDA = factory.createStructArray({1,numCustomOutputs}, fieldNames);

                // Copy custom output data to matlab::data::Array format to pass into MATLAB
                for(std::size_t k=0; k < numCustomOutputs; k++) {
                    lhsCustomTypesMDA[k]["OutputPosition"] = factory.createScalar<uint64_t>(lhsCustomTypes.at(k).OutputPosition);
                    lhsCustomTypesMDA[k]["CPPName"] = factory.createCharArray(lhsCustomTypes.at(k).CPPName);
                    lhsCustomTypesMDA[k]["LibraryName"] = factory.createCharArray(lhsCustomTypes.at(k).LibraryName);
                }

            }

            // Add info about outputs to the inputs. Will be used to inform feval to release ownwership of output C++ interface object and return underlying C++ pointer
            rhsList.insert(rhsList.begin(), lhsCustomTypesMDA);

            // Add the actual function name as extra input to right-hand-side
            matlab::data::Array functionName = factory.createCharArray(function);
            rhsList.insert(rhsList.begin(), functionName);

            // Call into modified feval because it can handle user-defined inputs then call normal feval as usual
            const std::u16string shimFunctionName = u"matlab.engine.internal.fevalWithCustomTypeConversion";

            size_t nrhs = rhsList.size();
            std::unique_ptr<matlab::data::impl::ArrayImpl*, void(*)(matlab::data::impl::ArrayImpl**)> argsImplPtr(new matlab::data::impl::ArrayImpl*[nrhs], [](matlab::data::impl::ArrayImpl** ptr) {
                delete[] ptr;
            });
            matlab::data::impl::ArrayImpl** argsImpl = argsImplPtr.get();
            size_t i = 0;
            for (auto e : rhsList) {
                argsImpl[i++] = matlab::data::detail::Access::getImpl<matlab::data::impl::ArrayImpl>(e);
            }

            auto const nlhs = detail::createLhs<ReturnType>::nlhs; // detect number of outputs

            FutureResult<std::vector<matlab::data::Array>> f = fevalAsync(shimFunctionName, nlhs, rhsList, output, error);

            // c++11 lambdas do not correctly handle move operations...
            // when c++14 is available, this should be:
            // auto convertToResultType = [copyableF = std::move(f)]()->ReturnType { ....... };
            auto copyableF = std::make_shared<FutureResult<std::vector<matlab::data::Array>>>(std::move(f));
            auto convertToResultType = [copyableF]() ->ReturnType {
                std::vector<matlab::data::Array> vec = copyableF->get();
                detail::createLhs<ReturnType> lhsFactory;
                return lhsFactory(std::move(vec));
            };

            std::future<ReturnType> future = std::async(std::launch::deferred, std::move(convertToResultType));
            return FutureResult<ReturnType>(std::move(future), copyableF->getTaskReference());
        }

        template<class ReturnType, typename... RhsArgs>
        FutureResult<ReturnType> ExecutionInterface::fevalAsync(const std::string &function,
            const std::shared_ptr<StreamBuffer> &output,
            const std::shared_ptr<StreamBuffer> &error,
            RhsArgs&&... rhsArgs
            ) {
            return convertUTF8StringToUTF16String(std::u16string(function.cbegin(), function.cend()),
                output, error, std::forward<RhsArgs>(rhsArgs)...);
        }

        template<class ReturnType, typename... RhsArgs>
        FutureResult<ReturnType> ExecutionInterface::fevalAsync(const std::u16string &function,
            RhsArgs&&... rhsArgs
            ) {
            const std::shared_ptr<StreamBuffer> defaultBuffer;
            return fevalAsync<ReturnType>(function, defaultBuffer, defaultBuffer, std::forward<RhsArgs>(rhsArgs)...);
        }

        template<class ReturnType, typename... RhsArgs>
        FutureResult<ReturnType> ExecutionInterface::fevalAsync(const std::string &function,
            RhsArgs&&... rhsArgs
            ) {
            return fevalAsync<ReturnType>(std::u16string(function.cbegin(), function.cend()),
                std::forward<RhsArgs>(rhsArgs)...);
        }

        inline std::string ExecutionInterface::convertUTF16StringToASCIIString(const std::u16string &str)
        {
            std::unique_ptr<char []> asciistr_ptr(new char[str.size()+1]);
            asciistr_ptr.get()[str.size()] = '\0';
            const char* u16_src = reinterpret_cast<const char*>(str.c_str());
            for(size_t n = 0; n < str.size(); ++n)
            {
               asciistr_ptr.get()[n] = u16_src[2*n];
            }
            return std::string(asciistr_ptr.get());
        }


        inline void writeToStreamBuffer(void* buffer, const char16_t* stream, size_t n) {
            std::shared_ptr<StreamBuffer>* output = reinterpret_cast<std::shared_ptr<StreamBuffer>*>(buffer);
            output->get()->sputn(stream, n);
        }

        inline void deleteStreamBufferImpl(void* impl) {
            delete static_cast<std::shared_ptr<StreamBuffer>*>(impl);
        }
    }
}

#endif /* MATLAB_EXECUTION_INTERFACE_IMPL_HPP */