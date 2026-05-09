from pathlib import Path
import argparse

def srec_checksum(record: str) -> str:
    """
    Compute checksum for S-record body (everything after 'Sx').
    Input: hex string like "18500000...."
    Returns: 2-digit checksum string.
    """
    bytes_data = [int(record[i:i+2], 16) for i in range(0, len(record), 2)]
    checksum = (~sum(bytes_data) & 0xFF)
    return f"{checksum:02X}"


def pad_srec(input_file: str, output_file: str) -> None:
    """
    Pad S0 and S5 records with FF (keeping their checksum),
    drop S7, and pad last S3 record with FF + recalc checksum.
    Writes the result to output_file.
    """
    with open(input_file, "r") as f:
        lines = [line.strip() for line in f if line.strip()]

    # Find max length among normal S3 records
    s3_lengths = [len(line) for line in lines if line.startswith("S3")]
    target_len = max(s3_lengths) if s3_lengths else None

    # Find the index of the last S3 line
    last_s3_index = max(i for i, l in enumerate(lines) if l.startswith("S3"))

    new_lines = []
    for idx, line in enumerate(lines):
        if line.startswith("S7"):
            continue  # drop S7 lines

        elif line.startswith("S0") or line.startswith("S5"):
            # Pad with FF but keep original checksum
            prefix = line[:-2]
            checksum = line[-2:]
            while len(prefix) + 2 < target_len:
                prefix += "FF"
            new_lines.append(prefix + checksum)

        elif line.startswith("S3") and idx == last_s3_index:
            # Last S3 line → pad and recalc CRC
            rec_type = line[:2]
            body = line[2:-2]  # exclude 'S3' and checksum
            while len(rec_type + body + line[-2:]) < target_len:
                body += "FF"
            new_crc = srec_checksum(body)
            new_lines.append(rec_type + body + new_crc)

        else:
            new_lines.append(line)

    of = Path(output_file)

    with open(of, "w") as f:
        for l in new_lines:
            f.write(l + "\n")

    print(f"Padded SREC written to {of}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Pad and fix checksums in S-record files.")
    parser.add_argument("input_file", help="Path to input SREC file")
    parser.add_argument("output_file", help="Path to output padded SREC file")
    args = parser.parse_args()

    pad_srec(args.input_file, args.output_file)
