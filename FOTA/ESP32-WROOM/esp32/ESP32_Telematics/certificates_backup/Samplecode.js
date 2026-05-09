#!/usr/bin/env node

/**
 * AWS IoT Core Vehicle Data Connector
 * Simplified SDV POC Implementation
 * 
 * This application connects a vehicle to AWS IoT Core and sends basic telemetry data
 * with static data from JSON file and OTA update capabilities.
 */

const mqtt = require('aws-iot-device-sdk-v2');
const { v4: uuidv4 } = require('uuid');
const chalk = require('chalk');
const VehicleDataSimulator = require('./simulator/vehicleDataSimulator');
const MQTTManager = require('./mqtt/mqttManager');
const ConfigManager = require('./config/configManager');
const CertificateManager = require('./security/certificateManager');

class VehicleConnector {    
    constructor() {
        this.config = null;
        this.mqttManager = null;
        this.vehicleSimulator = null;
        this.isConnected = false;
        this.sessionId = uuidv4();
        this.startTime = new Date();

        // Basic telemetry interval
        this.basicTelemetryInterval = null;

        // OTA package management - Load version from package.json
        this.currentVersion = this.loadCurrentVersion();
        this.otaPackageInfo = null;
        this.downloadInProgress = false;

        // Rollback and failure detection properties
        this.rollbackInProgress = false;
        this.lastBackupPath = null;
        this.previousVersion = null;
        this.updateStartTime = null;
        this.maxStartupTime = 30000; // 30 seconds max for app to start after update
        this.healthCheckInterval = null;
        this.healthCheckAttempts = 0;
        this.maxHealthCheckAttempts = 6; // 3 minutes of health checks
        this.currentPhase = 'basic';
    }

    loadCurrentVersion() {
        try {
            const path = require('path');
            const fs = require('fs');
            const packageJsonPath = path.join(__dirname, '../package.json');
            const packageJson = JSON.parse(fs.readFileSync(packageJsonPath, 'utf8'));
            return packageJson.version;
        } catch (error) {
            console.warn(`⚠️ Could not load version from package.json: ${error.message}`);
            return '1.0.0'; // Fallback version
        }
    }

    async initialize() {
        try {
            console.log(chalk.blue('🚗 AWS IoT Vehicle Connector - Simplified SDV POC'));
            console.log(chalk.gray('='.repeat(50)));
            
            // Load configuration
            this.config = await ConfigManager.loadConfig();
            console.log(chalk.green('✓ Configuration loaded'));

            // Validate certificates
            const certManager = new CertificateManager(this.config);
            await certManager.validateCertificates();
            console.log(chalk.green('✓ Certificates validated'));

            // Initialize vehicle simulator
            this.vehicleSimulator = new VehicleDataSimulator(this.config);
            console.log(chalk.green('✓ Vehicle simulator initialized'));

            // Initialize MQTT manager
            this.mqttManager = new MQTTManager(this.config);
            await this.mqttManager.initialize();
            console.log(chalk.green('✓ MQTT manager initialized'));

            // Setup event handlers
            this.setupEventHandlers();

            // Check if this is a startup after OTA update and perform health check
            await this.performStartupHealthCheck();

            console.log(chalk.yellow(`🌐 IoT Endpoint: ${this.config.aws.iotEndpoint}`));

        } catch (error) {
            console.error(chalk.red('❌ Initialization failed:'), error.message);
            
            // If initialization fails during OTA update, trigger rollback
            if (await this.isRecentOTAUpdate()) {
                console.log(chalk.red('🔄 Initialization failure detected after OTA update - triggering rollback...'));
                await this.initiateAutomaticRollback('initialization_failed', error.message);
            }
            
            throw error;
        }
    }

    setupEventHandlers() {
        // MQTT connection events
        this.mqttManager.on('connected', () => {
            this.isConnected = true;
            console.log(chalk.green('\n🔗 Connected to AWS IoT Core'));
            this.startBasicTelemetry();
            // Subscribe to OTA notifications after connection
            this.subscribeToOTANotifications();
        });

        this.mqttManager.on('disconnected', () => {
            this.isConnected = false;
            console.log(chalk.red('\n🔌 Disconnected from AWS IoT Core'));
            this.stopAllTelemetry();
        });

        this.mqttManager.on('error', (error) => {
            console.error(chalk.red('❌ MQTT Error:'), error.message);
            
            // If MQTT errors persist after recent OTA update, consider rollback
            if (this.isRecentOTAUpdate()) {
                console.log(chalk.yellow('⚠️ MQTT errors detected after recent OTA update'));
                // Could trigger rollback if errors persist
            }
        });        
        
        // Add process-level error handlers for rollback scenarios
        this.setupProcessErrorHandlers();
    }

    /**
     * Setup process-level error handlers for automatic rollback
     */
    setupProcessErrorHandlers() {
        // Handle uncaught exceptions
        process.on('uncaughtException', async (error) => {
            console.error(chalk.red('❌ Uncaught Exception:'), error.message);
            console.error(error.stack);
            
            if (await this.isRecentOTAUpdate() && !this.rollbackInProgress) {
                console.log(chalk.red('🔄 Uncaught exception after OTA update - initiating rollback...'));
                try {
                    await this.initiateAutomaticRollback('uncaught_exception', error.message);
                } catch (rollbackError) {
                    console.error(chalk.red('❌ Rollback failed after uncaught exception'));
                    process.exit(1);
                }
            } else {
                process.exit(1);
            }
        });

        // Handle unhandled promise rejections
        process.on('unhandledRejection', async (reason, promise) => {
            console.error(chalk.red('❌ Unhandled Promise Rejection:'), reason);
            
            if (await this.isRecentOTAUpdate() && !this.rollbackInProgress) {
                console.log(chalk.red('🔄 Unhandled rejection after OTA update - initiating rollback...'));
                try {
                    await this.initiateAutomaticRollback('unhandled_rejection', reason?.message || 'Unknown rejection');
                } catch (rollbackError) {
                    console.error(chalk.red('❌ Rollback failed after unhandled rejection'));
                    process.exit(1);
                }
            }
        });

        // Handle SIGTERM gracefully with rollback consideration
        process.on('SIGTERM', async () => {
            console.log(chalk.yellow('📡 Received SIGTERM'));
            
            // Stop health monitoring
            this.stopHealthMonitoring();
            
            await this.disconnect();
            process.exit(0);
        });

        // Handle SIGINT gracefully with rollback consideration  
        process.on('SIGINT', async () => {
            console.log(chalk.yellow('📡 Received SIGINT'));
            
            // Stop health monitoring
            this.stopHealthMonitoring();
            
            await this.disconnect();
            process.exit(0);
        });
    }

    async startBasicTelemetry() {
        console.log(chalk.cyan('\n📡 Starting basic telemetry transmission...'));
        
        // Start basic telemetry (every 5 seconds)
        console.log(chalk.blue(`\n🚀 Basic Telemetry Started`));
        this.basicTelemetryInterval = setInterval(() => {
            this.sendBasicTelemetry();
        }, 5000);
    }

    async sendBasicTelemetry() {
        if (!this.isConnected) return;

        const telemetryData = this.vehicleSimulator.generateBasicTelemetry();
        const topic = `${this.config.mqtt.topicPrefix}/${this.config.vehicle.id}/telemetry/basic`;

        const message = {
            vehicleId: this.config.vehicle.id,
            vin: this.config.vehicle.vin,
            timestamp: new Date().toISOString(),
            sessionId: this.sessionId,
            phase: 'basic',
            data: telemetryData
        };       
         await this.mqttManager.publish(topic, message);        
        // Log speed violations in console for visibility
        if (telemetryData.isViolating) {
            console.log(chalk.red(`🚨 SPEED VIOLATION: ${telemetryData.speed}km/h exceeds limit of ${telemetryData.speedLimit || 'N/A'}km/h - Severity: ${telemetryData.severity}`));
        } else {
            console.log(chalk.cyan(`📊 Basic telemetry sent - Speed: ${telemetryData.speed}km/h, Fuel: ${telemetryData.fuelLevel}%`));
        }
    }   
    
    async subscribeToOTANotifications() {
        if (!this.isConnected) {
            console.warn(chalk.yellow('⚠️ Cannot subscribe to OTA notifications - not connected'));
            return false;
        }

        try {
            console.log(chalk.blue('\n📦 Setting up OTA package notification subscription...'));

            // OTA package notification topic (from Lambda → IoT Core)
            const otaNotificationTopic = `sdv/vehicles/${this.config.vehicle.id}/notifications/s3`;
            await this.mqttManager.subscribe(otaNotificationTopic, (topic, message) => {
                console.log(chalk.gray(`OTA Package: ${JSON.stringify(message, null, 2)}`));
                this.handleOTAPackageNotification(topic, message);
            }, 1); // QoS 1 for reliable delivery

            console.log(chalk.green('✅ OTA notification subscription established:'));
            console.log(chalk.gray(`   📦 Topic: ${otaNotificationTopic}`));

            return true;
        } catch (error) {
            console.error(chalk.red('❌ Failed to subscribe to OTA notifications:'), error.message);
            return false;
        }
    }

    handleOTAPackageNotification(topic, message) {
        try {
            console.log(chalk.blue('\n📦 OTA Package Notification Received'));
            console.log(chalk.gray(`Topic: ${topic}`));
            console.log(chalk.gray(`url: ${message.package.url}, version: ${message.package.version}`));

            // Validate notification structure
            if (!message.package.url || !message.package.version) {
                console.warn(chalk.yellow('⚠️ Invalid OTA package notification format'));
                this.sendOTANotificationResponse(message, 'invalid_ota_package');
                return;
            }

            console.log(chalk.cyan(`📏 Package Size: ${message.package.size ? this.formatFileSize(message.package.size) : 'Unknown'}`));

            // Check if we need to update
            if (this.shouldUpdateVersion(message.package.version)) {
                this.processOTAUpdate(message.package);
            } else {
                console.log(chalk.yellow(`⚠️ Version ${message.package.version} is not newer than current ${this.currentVersion}`));
                this.sendOTANotificationResponse(message, 'version_not_newer');
            }

        } catch (error) {
            console.error(chalk.red('❌ Error handling OTA package notification:'), error.message);
        }
    }

    shouldUpdateVersion(newVersion) {
        // Simple version comparison (assumes semantic versioning)
        const current = this.currentVersion.split('.').map(Number);
        const incoming = newVersion.split('.').map(Number);

        for (let i = 0; i < Math.max(current.length, incoming.length); i++) {
            const c = current[i] || 0;
            const n = incoming[i] || 0;
            if (n > c) return true;
            if (n < c) return false;
        }
        return false;
    }

    async processOTAUpdate(packageInfo) {
        try {
            console.log(chalk.blue(`\n🔄 Processing OTA update to version ${packageInfo.version}...`));

            // Store package info
            this.otaPackageInfo = {
                version: packageInfo.version,
                packageUrl: packageInfo.url,
                size: packageInfo.size,
                checksum: packageInfo.checksum,
                timestamp: new Date().toISOString()
            };

            console.log(chalk.yellow(`package info - ${JSON.stringify(this.otaPackageInfo, null, 2)}`));

            // Send acknowledgment
            this.sendOTANotificationResponse(packageInfo, 'download_starting');
            
            // Start download process
            await this.downloadOTAPackage();

        } catch (error) {
            console.error(chalk.red('❌ Error processing OTA update:'), error.message);
            this.sendOTANotificationResponse(packageInfo, 'download_failed', error.message);
        }
    }

    async downloadOTAPackage() {
        const fs = require('fs').promises;
        const https = require('https');
        const path = require('path');

        try {
            console.log(chalk.yellow('📥 Starting real OTA package download...'));
            this.downloadInProgress = true;

            // Create downloads directory
            const downloadDir = path.join(__dirname, '../downloads');
            await fs.mkdir(downloadDir, { recursive: true });

            const fileName = `vehicle-app-${this.otaPackageInfo.version}.tar.gz`;
            const filePath = path.join(downloadDir, fileName);

            // Download file from S3 using AWS CLI or direct HTTPS
            const downloadUrl = this.otaPackageInfo.packageUrl;
            console.log(chalk.cyan(`📡 Downloading from: ${downloadUrl}`));

            // Use AWS CLI for S3 downloads (more reliable)
            if (downloadUrl.includes('s3.amazonaws.com')) {
                const { exec } = require('child_process');
                const { promisify } = require('util');
                const execAsync = promisify(exec);

                const s3Path = downloadUrl.replace(/https:\/\/([^\/]+)\.s3\.amazonaws\.com\//, 's3://$1/');
                const downloadCommand = `aws s3 cp "${s3Path}" "${filePath}"`;

                await execAsync(downloadCommand);
                console.log(chalk.green('✅ Package downloaded via AWS CLI'));
            } else {
                // Direct HTTPS download
                await this.downloadFileHTTPS(downloadUrl, filePath);
                console.log(chalk.green('✅ Package downloaded via HTTPS'));
            }

            // Verify file size
            const stats = await fs.stat(filePath);
            console.log(chalk.cyan(`📊 Downloaded size: ${this.formatFileSize(stats.size)}`));

            this.downloadInProgress = false;
            await this.completeOTADownload();

        } catch (error) {
            console.error(chalk.red('❌ Download failed:'), error.message);
            this.downloadInProgress = false;
            await this.sendOTANotificationResponse(this.otaPackageInfo, 'download_failed', error.message);
        }
    }

    downloadFileHTTPS(url, filePath) {
        const fs = require('fs');
        const https = require('https');

        return new Promise((resolve, reject) => {
            const file = fs.createWriteStream(filePath);
            
            https.get(url, (response) => {
                if (response.statusCode !== 200) {
                    reject(new Error(`HTTP ${response.statusCode}: ${response.statusMessage}`));
                    return;
                }

                response.pipe(file);

                file.on('finish', () => {
                    file.close();
                    resolve();
                });

                file.on('error', reject);
            }).on('error', reject);
        });
    }    
    async completeOTADownload() {
        try {
            console.log(chalk.green('\n✅ OTA package download completed'));

            // Send download completion notification
            await this.sendOTANotificationResponse(this.otaPackageInfo, 'download_completed');

            // Apply the update (extraction will happen inside applyOTAUpdate)
            await this.applyOTAUpdate();

        } catch (error) {
            console.error(chalk.red('❌ Error completing OTA download:'), error.message);
            await this.sendOTANotificationResponse(this.otaPackageInfo, 'update_failed', error.message);
        }
    }    
    async extractOTAPackage() {
        const { exec } = require('child_process');
        const { promisify } = require('util');
        const execAsync = promisify(exec);
        const path = require('path');

        try {
            console.log(chalk.yellow('📦 Extracting OTA package...'));

            const downloadDir = path.join(__dirname, '../downloads');
            const extractDir = path.join(downloadDir, 'extracted', this.otaPackageInfo.version);
            const fileName = `vehicle-app-${this.otaPackageInfo.version}.tar.gz`;
            const filePath = path.join(downloadDir, fileName);

            // Create extraction directory
            const fs = require('fs').promises;
            await fs.mkdir(extractDir, { recursive: true });

            // Extract the tar.gz file
            const extractCommand = `tar -xzf "${filePath}" -C "${extractDir}"`;
            await execAsync(extractCommand);

            console.log(chalk.green('✅ Package extracted successfully'));
            console.log(chalk.gray(`   📁 Extracted to: ${extractDir}`));

            // Check if we have a nested directory structure and return the correct path
            const extractedContents = await fs.readdir(extractDir);
            if (extractedContents.length === 1) {
                const nestedPath = path.join(extractDir, extractedContents[0]);
                const nestedStats = await fs.stat(nestedPath);
                if (nestedStats.isDirectory()) {
                    console.log(chalk.cyan(`🔍 Found nested directory: ${extractedContents[0]}`));
                    return nestedPath;
                }
            }

            return extractDir;

        } catch (error) {
            console.error(chalk.red('❌ Extraction failed:'), error.message);
            throw error;
        }
    }    
    async applyOTAUpdate() {
        try {
            console.log(chalk.magenta('🔧 Starting OTA update process...'));

            // Send update starting notification
            await this.sendOTANotificationResponse(this.otaPackageInfo, 'update_starting');

            // STEP 1: Stop IoT connection before file replacement
            console.log(chalk.yellow('🔌 Stopping IoT connection for update...'));
            await this.mqttManager.disconnect();
            this.isConnected = false;
            this.stopAllTelemetry();

            // STEP 2: Extract downloaded package
            console.log(chalk.blue('📦 Extracting OTA package...'));
            const extractPath = await this.extractOTAPackage();

            // STEP 3: Backup current application
            console.log(chalk.blue('💾 Creating backup of current application...'));
            await this.backupCurrentApplication();
            //return // for now
            // STEP 4: Replace application files
            console.log(chalk.blue('📁 Replacing application files...'));
            await this.replaceApplicationFiles(extractPath);

            // STEP 5: Update version information
            console.log(chalk.blue('🏷️ Updating version information...'));
            await this.updateVersionFile();

            // STEP 6: Restart the application process
            console.log(chalk.magenta('🔄 Restarting application with new version...'));
            await this.restartApplication();

        } catch (error) {
            console.error(chalk.red('❌ OTA update failed:'), error.message);

            // Automatic rollback on OTA update failure
            console.log(chalk.red('🔄 OTA update failed - initiating automatic rollback...'));
            
            try {
                await this.initiateAutomaticRollback('ota_update_failed', error.message);
            } catch (rollbackError) {
                console.error(chalk.red('❌ Both OTA update and rollback failed:'), rollbackError.message);
                
                // Try to reconnect on catastrophic failure
                try {
                    await this.mqttManager.connect();
                    this.isConnected = true;
                    await this.sendOTANotificationResponse(this.otaPackageInfo, 'critical_failure', 
                        `Update failed: ${error.message}. Rollback failed: ${rollbackError.message}`);
                } catch (reconnectError) {
                    console.error(chalk.red('❌ Failed to reconnect after catastrophic failure'));
                }
            }
        }
    }

    async backupCurrentApplication() {
        const fs = require('fs').promises;
        const path = require('path');

        try {
            // Create backup directory OUTSIDE the application directory
            const backupPath = path.join(__dirname, '../../backups', `backup-${this.currentVersion}-${Date.now()}`);
            const currentAppPath = path.join(__dirname, '..');

            console.log(chalk.cyan(`🏗️ Creating backup directory: ${backupPath}`));
            console.log(chalk.cyan(`📁 Source directory: ${currentAppPath}`));

            // Create backup directory
            await fs.mkdir(backupPath, { recursive: true });

            // Copy files/directories manually using Node.js fs (more reliable)
            const itemsToBackup = [
                { name: 'src', type: 'directory' },
                { name: 'config', type: 'directory' },
                { name: 'package.json', type: 'file' },
                { name: 'package-lock.json', type: 'file' },
                { name: '.env', type: 'file' },
                { name: 'README.md', type: 'file' }
            ];

            for (const item of itemsToBackup) {
                const sourcePath = path.join(currentAppPath, item.name);
                const targetPath = path.join(backupPath, item.name);

                try {
                    // Check if source exists
                    await fs.access(sourcePath);

                    if (item.type === 'directory') {
                        // Copy directory recursively
                        await this.copyDirectory(sourcePath, targetPath);
                        console.log(chalk.green(`✅ Backed up directory: ${item.name}`));
                    } else {
                        // Copy single file
                        await fs.copyFile(sourcePath, targetPath);
                        console.log(chalk.green(`✅ Backed up file: ${item.name}`));
                    }

                } catch (error) {
                    console.log(chalk.yellow(`⚠️ Skipping ${item.name} - ${error.code === 'ENOENT' ? 'not found' : 'inaccessible'}`));
                }
            }

            console.log(chalk.green(`✅ Backup created at: ${backupPath}`));
            
            // Store backup path for potential rollback
            this.lastBackupPath = backupPath;
            
            // Store previous version for rollback purposes
            this.previousVersion = this.currentVersion;
            
            return backupPath;

        } catch (error) {
            throw new Error(`Failed to create backup: ${error.message}`);
        }
    }

    async copyDirectory(src, dest) {
        const fs = require('fs').promises;
        const path = require('path');

        try {
            // Create destination directory
            await fs.mkdir(dest, { recursive: true });

            // Read source directory contents
            const entries = await fs.readdir(src, { withFileTypes: true });

            for (const entry of entries) {
                const srcPath = path.join(src, entry.name);
                const destPath = path.join(dest, entry.name);

                if (entry.isDirectory()) {
                    // Recursively copy subdirectory
                    await this.copyDirectory(srcPath, destPath);
                } else {
                    // Copy file
                    await fs.copyFile(srcPath, destPath);
                }
            }
        } catch (error) {
            console.log(chalk.yellow(`⚠️ Warning copying directory ${src}: ${error.message}`));
        }
    }

    async replaceApplicationFiles(extractPath) {
        const fs = require('fs').promises;
        const path = require('path');

        try {
            const currentAppPath = path.join(__dirname, '..');

            // List of files/directories to replace
            const filesToReplace = [
                { name: 'src', type: 'directory' },
                { name: 'config', type: 'directory' },
                { name: 'package.json', type: 'file' },
                { name: 'package-lock.json', type: 'file' }                
            ];

            console.log(chalk.cyan(`📁 Extract path: ${extractPath}`));
            console.log(chalk.cyan(`📁 Target path: ${currentAppPath}`));

            for (const item of filesToReplace) {
                const sourcePath = path.join(extractPath, item.name);
                const targetPath = path.join(currentAppPath, item.name);

                console.log(chalk.cyan(`🔄 Processing ${item.name}...`));
                console.log(chalk.gray(`   Source: ${sourcePath}`));
                console.log(chalk.gray(`   Target: ${targetPath}`));

                // Check if source exists in extracted package
                try {
                    await fs.access(sourcePath);
                    console.log(chalk.green(`✅ Found ${item.name} in update package`));
                } catch {
                    console.log(chalk.yellow(`⚠️ Skipping ${item.name} - not found in update package`));
                    continue;
                }

                // Remove existing file/directory
                try {
                    console.log(chalk.cyan(`🗑️ Removing existing ${item.name}...`));
                    await fs.rm(targetPath, { recursive: true, force: true });
                    console.log(chalk.green(`✅ Removed existing ${item.name}`));
                } catch (error) {
                    console.log(chalk.yellow(`⚠️ Could not remove existing ${item.name}: ${error.message}`));
                    // Continue anyway - file might not exist
                }

                // Copy new file/directory using Node.js fs
                try {
                    if (item.type === 'directory') {
                        console.log(chalk.cyan(`📁 Copying directory ${item.name}...`));
                        await this.copyDirectory(sourcePath, targetPath);
                    } else {
                        console.log(chalk.cyan(`📄 Copying file ${item.name}...`));
                        await fs.copyFile(sourcePath, targetPath);
                    }
                    console.log(chalk.green(`✅ Replaced: ${item.name}`));
                } catch (error) {
                    console.error(chalk.red(`❌ Failed to copy ${item.name}: ${error.message}`));
                    throw new Error(`Failed to replace ${item.name}: ${error.message}`);
                }
            }

            console.log(chalk.green('✅ All application files replaced successfully'));

        } catch (error) {
            console.error(chalk.red(`❌ File replacement failed: ${error.message}`));
            throw new Error(`Failed to replace application files: ${error.message}`);
        }
    }

    async updateVersionFile() {
        const fs = require('fs').promises;
        const path = require('path');

        try {
            const packageJsonPath = path.join(__dirname, '../package.json');

            // Read current package.json
            const packageJson = JSON.parse(await fs.readFile(packageJsonPath, 'utf8'));

            // Update version
            packageJson.version = this.otaPackageInfo.version;
            packageJson.lastOTAUpdate = new Date().toISOString();

            // Write updated package.json
            await fs.writeFile(packageJsonPath, JSON.stringify(packageJson, null, 2));

            // Update current version in memory
            this.currentVersion = this.otaPackageInfo.version;

            console.log(chalk.green(`✅ Version updated to: ${this.currentVersion}`));

        } catch (error) {
            throw new Error(`Failed to update version file: ${error.message}`);
        }
    }

    async restartApplication() {
        const { spawn } = require('child_process');
        const path = require('path');

        try {
            console.log(chalk.yellow('🔄 Initiating application restart...'));

            // Reconnect to MQTT before sending completion notification
            try {
                if (!this.isConnected) {
                    console.log(chalk.cyan('🔗 Reconnecting to MQTT to send completion notification...'));
                    await this.mqttManager.connect();
                    this.isConnected = true;
                    // Wait a moment for connection to stabilize
                    await new Promise(resolve => setTimeout(resolve, 2000));
                }

                // Send final status before restart
                await this.sendOTANotificationResponse(this.otaPackageInfo, 'update_completed');
                console.log(chalk.green('📤 OTA Update completed successfully'));

                // Wait for the message to be sent
                await new Promise(resolve => setTimeout(resolve, 1000));

            } catch (mqttError) {
                console.log(chalk.yellow(`⚠️ Could not send completion notification: ${mqttError.message}`));
                // Continue with restart anyway
            }

            // Create robust restart script with better error handling
            const restartScript = process.platform === 'win32' ? `
@echo off
echo ========================================
echo 🔄 OTA UPDATE RESTART SCRIPT
echo ========================================
echo 📦 Current directory: %CD%
echo 🕐 Time: %DATE% %TIME%
echo.

REM Change to application directory
cd /d "${path.join(__dirname, '..')}"
echo 📁 Changed to application directory: %CD%

echo.
echo 📦 Installing dependencies...
call npm install --production 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ❌ NPM install failed with error level %ERRORLEVEL%
    echo 📋 Attempting to continue anyway...
    echo.
)

echo.
echo 🚀 Starting application with npm start...
echo ========================================
call npm start
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ❌ Application failed to start with error level %ERRORLEVEL%
    echo 🔄 Trying direct node command as fallback...
    node src/vehicleConnector.js
)
echo.
echo ⚠️ Script completed - press any key to close window
pause
        ` : `
#!/bin/bash
echo "========================================"
echo "🔄 OTA UPDATE RESTART SCRIPT"
echo "========================================"
echo "📦 Current directory: $(pwd)"
echo "🕐 Time: $(date)"
echo ""

# Change to application directory
cd "${path.join(__dirname, '..')}"
echo "📁 Changed to application directory: $(pwd)"

echo ""
echo "📦 Installing dependencies..."
npm install --production
if [ $? -ne 0 ]; then
    echo ""
    echo "❌ NPM install failed with exit code $?"
    echo "📋 Attempting to continue anyway..."
    echo ""
fi

echo ""
echo "🚀 Starting application with npm start..."
echo "========================================"
npm start
if [ $? -ne 0 ]; then
    echo ""
    echo "❌ Application failed to start with exit code $?"
    echo "🔄 Trying direct node command as fallback..."
    node src/vehicleConnector.js
fi
        `;

            const fs = require('fs').promises;
            const restartScriptPath = path.join(__dirname, '../ota-restart.' + (process.platform === 'win32' ? 'bat' : 'sh'));

            console.log(chalk.cyan(`📝 Creating restart script: ${restartScriptPath}`));
            await fs.writeFile(restartScriptPath, restartScript);

            // Set executable permissions on Unix-like systems
            if (process.platform !== 'win32') {
                await fs.chmod(restartScriptPath, '755');
            }

            console.log(chalk.magenta('🚀 Executing restart script...'));

            // Better command execution for Windows
            const command = process.platform === 'win32' ? 'cmd' : 'bash';
            const args = process.platform === 'win32'
                ? ['/c', 'start', '"OTA Restart"', '/wait', `"${restartScriptPath}"`]
                : [restartScriptPath];

            console.log(chalk.cyan(`🔧 Executing command: ${command} ${args.join(' ')}`));

            const child = spawn(command, args, {
                detached: true,
                stdio: 'ignore',
                cwd: path.join(__dirname, '..'),
                shell: true // Use shell for better Windows compatibility
            });

            // Detach the child process
            child.unref();

            console.log(chalk.green('✅ Restart script launched'));
            console.log(chalk.magenta('🔄 Current process will exit in 5 seconds...'));

            // Longer delay to ensure script starts
            setTimeout(() => {
                console.log(chalk.magenta('🔄 OTA Update completed - Process exiting for restart...'));
                process.exit(0);
            }, 5000);

        } catch (error) {
            console.error(chalk.red(`❌ Restart failed: ${error.message}`));

            // Try to send failure notification if possible
            try {
                if (this.isConnected) {
                    await this.sendOTANotificationResponse(this.otaPackageInfo, 'update_failed', error.message);
                }
            } catch (notifyError) {
                console.error(chalk.red(`❌ Failed to send error notification: ${notifyError.message}`));
            }

            throw new Error(`Failed to restart application: ${error.message}`);
        }
    }    
    async sendOTANotificationResponse(packageInfo, status, errorMessage = null) {
        try {
            const responseTopic = `sdv/vehicles/${this.config.vehicle.id}/notifications/s3/response`;
            console.log(`inside sendOTANotificationResponse, status:${status}, topic:${responseTopic}`);

            const responseMessage = {
                vehicleId: this.config.vehicle.id,
                vin: this.config.vehicle.vin,
                timestamp: new Date().toISOString(),
                sessionId: this.sessionId,
                currentVersion: this.currentVersion,
                packageVersion: packageInfo.version || packageInfo.package?.version,
                status: status,
                packageUrl: packageInfo.packageUrl || packageInfo.package?.url,
                currentPhase: this.currentPhase,
                downloadInProgress: this.downloadInProgress
            };            
            if (errorMessage) {
                responseMessage.error = errorMessage;
            }

            await this.mqttManager.publish(responseTopic, responseMessage, 1);
            console.log(chalk.gray(`📤 Sent OTA notification response: ${status}`));

        } catch (error) {
            console.error(chalk.red('❌ Failed to send OTA notification response:'), error.message);
        }
    }

    formatFileSize(bytes) {
        if (bytes === 0) return '0 Bytes';
        const k = 1024;
        const sizes = ['Bytes', 'KB', 'MB', 'GB'];
        const i = Math.floor(Math.log(bytes) / Math.log(k));
        return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
    }

    // === UTILITY METHODS ===

    stopAllTelemetry() {
        console.log(chalk.yellow('\n⏹️ Stopping all telemetry...'));

        if (this.basicTelemetryInterval) {
            clearInterval(this.basicTelemetryInterval);
            this.basicTelemetryInterval = null;
            console.log(chalk.gray('✓ Basic telemetry stopped'));
        }
        
        // Stop health monitoring if running
        this.stopHealthMonitoring();
    }

    async connect() {
        try {
            await this.mqttManager.connect();
        } catch (error) {
            console.error(chalk.red('❌ Connection failed:'), error.message);
            throw error;
        }
    }

    async disconnect() {
        try {
            console.log(chalk.yellow('\n🔌 Disconnecting...'));

            // Stop all telemetry
            this.stopAllTelemetry();

            // Disconnect MQTT
            if (this.mqttManager) {
                await this.mqttManager.disconnect();
            }

            console.log(chalk.green('✅ Disconnected successfully'));

        } catch (error) {
            console.error(chalk.red('❌ Disconnect error:'), error.message);
            throw error;
        }
    }

    // === ROLLBACK SYSTEM ===

    /**
     * Check if this startup is shortly after an OTA update
     */
    async isRecentOTAUpdate() {
        try {
            const fs = require('fs').promises;
            const path = require('path');
            const packageJsonPath = path.join(__dirname, '../package.json');
            
            const packageJson = JSON.parse(await fs.readFile(packageJsonPath, 'utf8'));
            
            if (packageJson.lastOTAUpdate) {
                const updateTime = new Date(packageJson.lastOTAUpdate);
                const now = new Date();
                const timeDiff = now - updateTime;
                
                // Consider it recent if within last 5 minutes
                return timeDiff < (5 * 60 * 1000);
            }
            
            return false;
        } catch (error) {
            return false;
        }
    }

    /**
     * Perform health check after startup to detect OTA-related issues
     */
    async performStartupHealthCheck() {
        try {
            console.log(chalk.cyan('🔍 Performing startup health check...'));
            
            // Check if this is shortly after an OTA update
            const isRecentUpdate = await this.isRecentOTAUpdate();
            
            if (isRecentUpdate) {
                console.log(chalk.yellow('⚠️ Recent OTA update detected - performing enhanced health check...'));
                this.updateStartTime = new Date();
                
                // Start continuous health monitoring
                this.startPostUpdateHealthMonitoring();
            } else {
                console.log(chalk.green('✅ Normal startup - no recent OTA update detected'));
            }
            
        } catch (error) {
            console.warn(chalk.yellow(`⚠️ Health check warning: ${error.message}`));
        }
    }

    /**
     * Start continuous health monitoring after OTA update
     */
    startPostUpdateHealthMonitoring() {
        console.log(chalk.cyan('🔍 Starting post-update health monitoring...'));
        
        this.healthCheckAttempts = 0;
        
        this.healthCheckInterval = setInterval(async () => {
            this.healthCheckAttempts++;
            
            console.log(chalk.gray(`🔍 Health check ${this.healthCheckAttempts}/${this.maxHealthCheckAttempts}...`));
            
            try {
                const isHealthy = await this.performHealthCheck();
                
                if (isHealthy) {
                    console.log(chalk.green('✅ Health check passed - OTA update successful!'));
                    this.stopHealthMonitoring();
                    await this.notifyOTASuccess();
                } else if (this.healthCheckAttempts >= this.maxHealthCheckAttempts) {
                    console.log(chalk.red('❌ Health check failed after maximum attempts - initiating rollback...'));
                    this.stopHealthMonitoring();
                    await this.initiateAutomaticRollback('health_check_failed', 'Application failed health checks after OTA update');
                }
                
            } catch (error) {
                console.error(chalk.red(`❌ Health check error: ${error.message}`));
                
                if (this.healthCheckAttempts >= this.maxHealthCheckAttempts) {
                    this.stopHealthMonitoring();
                    await this.initiateAutomaticRollback('health_check_error', error.message);
                }
            }
            
        }, 30000); // Check every 30 seconds
    }

    /**
     * Perform individual health check
     */
    async performHealthCheck() {
        try {
            // Check 1: MQTT Connection
            if (!this.isConnected) {
                console.log(chalk.yellow('⚠️ Health check: MQTT not connected'));
                return false;
            }
            
            // Check 2: Vehicle Simulator
            if (!this.vehicleSimulator) {
                console.log(chalk.yellow('⚠️ Health check: Vehicle simulator not initialized'));
                return false;
            }
            
            // Check 3: Configuration
            if (!this.config) {
                console.log(chalk.yellow('⚠️ Health check: Configuration not loaded'));
                return false;
            }
            
            // Check 4: Send test telemetry to verify end-to-end functionality
            const testData = this.vehicleSimulator.generateBasicTelemetry();
            if (!testData || !testData.speed) {
                console.log(chalk.yellow('⚠️ Health check: Test telemetry generation failed'));
                return false;
            }
            
            console.log(chalk.green('✅ All health checks passed'));
            return true;
            
        } catch (error) {
            console.log(chalk.red(`❌ Health check failed: ${error.message}`));
            return false;
        }
    }

    /**
     * Stop health monitoring
     */
    stopHealthMonitoring() {
        if (this.healthCheckInterval) {
            clearInterval(this.healthCheckInterval);
            this.healthCheckInterval = null;
            console.log(chalk.gray('🔍 Health monitoring stopped'));
        }
    }

    /**
     * Notify successful OTA update
     */
    async notifyOTASuccess() {
        try {
            if (this.isConnected && this.otaPackageInfo) {
                await this.sendOTANotificationResponse(this.otaPackageInfo, 'update_verified_successful');
                console.log(chalk.green('📤 OTA success notification sent'));
            }
        } catch (error) {
            console.warn(chalk.yellow(`⚠️ Could not send OTA success notification: ${error.message}`));
        }
    }

    /**
     * Find the most recent backup for rollback
     */
    async findRollbackBackup() {
        try {
            const fs = require('fs').promises;
            const path = require('path');
            
            const backupsDir = path.join(__dirname, '../../backups');
            
            // Check if backups directory exists
            try {
                await fs.access(backupsDir);
            } catch {
                throw new Error('No backups directory found');
            }
            
            // Get all backup directories
            const entries = await fs.readdir(backupsDir, { withFileTypes: true });
            const backupDirs = entries
                .filter(entry => entry.isDirectory() && entry.name.startsWith('backup-'))
                .map(entry => ({
                    name: entry.name,
                    path: path.join(backupsDir, entry.name)
                }))
                .sort((a, b) => {
                    // Sort by timestamp (newest first)
                    const timestampA = a.name.split('-').pop();
                    const timestampB = b.name.split('-').pop();
                    return parseInt(timestampB) - parseInt(timestampA);
                });
            
            if (backupDirs.length === 0) {
                throw new Error('No backup directories found for rollback');
            }
            
            // Use the most recent backup
            const latestBackup = backupDirs[0];
            
            console.log(chalk.cyan(`🔍 Found rollback backup: ${latestBackup.name}`));
            
            // Validate backup contents
            const requiredFiles = ['src', 'package.json'];
            for (const file of requiredFiles) {
                const filePath = path.join(latestBackup.path, file);
                try {
                    await fs.access(filePath);
                } catch {
                    throw new Error(`Backup validation failed - missing ${file}`);
                }
            }
            
            console.log(chalk.green('✅ Backup validation successful'));
            
            return latestBackup;
            
        } catch (error) {
            throw new Error(`Failed to find rollback backup: ${error.message}`);
        }
    }

    /**
     * Extract version from backup directory name
     */
    extractVersionFromBackup(backupName) {
        try {
            // backup-1.0.0-1756206614692 -> 1.0.0
            const match = backupName.match(/backup-(.+)-\d+$/);
            return match ? match[1] : 'unknown';
        } catch {
            return 'unknown';
        }
    }

    /**
     * Initiate automatic rollback to previous version
     */
    async initiateAutomaticRollback(reason, errorMessage = null) {
        if (this.rollbackInProgress) {
            console.log(chalk.yellow('⚠️ Rollback already in progress - skipping'));
            return;
        }
        
        try {
            this.rollbackInProgress = true;
            
            console.log(chalk.red('\n🚨 AUTOMATIC ROLLBACK INITIATED'));
            console.log(chalk.red('='.repeat(50)));
            console.log(chalk.red(`🔴 Reason: ${reason}`));
            if (errorMessage) {
                console.log(chalk.red(`📝 Error: ${errorMessage}`));
            }
            console.log(chalk.red('='.repeat(50)));
            
            // Stop health monitoring if running
            this.stopHealthMonitoring();
            
            // Stop all telemetry
            this.stopAllTelemetry();
            
            // Disconnect MQTT to avoid conflicts during rollback
            if (this.isConnected) {
                try {
                    await this.mqttManager.disconnect();
                    this.isConnected = false;
                } catch (disconnectError) {
                    console.log(chalk.yellow(`⚠️ Could not disconnect cleanly: ${disconnectError.message}`));
                }
            }
            
            // Find and validate backup
            const backup = await this.findRollbackBackup();
            const rollbackVersion = this.extractVersionFromBackup(backup.name);
            
            console.log(chalk.yellow(`🔄 Rolling back to version: ${rollbackVersion}`));
            console.log(chalk.yellow(`📁 Using backup: ${backup.path}`));
            
            // Perform rollback
            await this.performRollback(backup, rollbackVersion);
            
            // Notify about rollback
            try {
                if (this.otaPackageInfo) {
                    // Try to reconnect briefly to send notification
                    await this.mqttManager.connect();
                    this.isConnected = true;
                    await this.sendOTANotificationResponse(this.otaPackageInfo, 'rollback_completed', `Rolled back due to: ${reason}`);
                    await this.mqttManager.disconnect();
                    this.isConnected = false;
                }
            } catch (notifyError) {
                console.log(chalk.yellow(`⚠️ Could not send rollback notification: ${notifyError.message}`));
            }
            
            // Restart with previous version
            await this.restartWithRollback(rollbackVersion);
            
        } catch (rollbackError) {
            console.error(chalk.red(`❌ Rollback failed: ${rollbackError.message}`));
            console.error(chalk.red('🚨 CRITICAL: System may be in unstable state'));
            
            // Log critical failure
            try {
                const fs = require('fs').promises;
                const path = require('path');
                const logPath = path.join(__dirname, '../rollback-failure.log');
                const logEntry = `${new Date().toISOString()} - ROLLBACK FAILURE\n` +
                                `Original Reason: ${reason}\n` +
                                `Original Error: ${errorMessage}\n` +
                                `Rollback Error: ${rollbackError.message}\n` +
                                `Current Version: ${this.currentVersion}\n\n`;
                await fs.appendFile(logPath, logEntry);
            } catch {
                // Even logging failed
            }
            
            throw rollbackError;
        } finally {
            this.rollbackInProgress = false;
        }
    }

    /**
     * Perform the actual rollback process
     */
    async performRollback(backup, rollbackVersion) {
        const fs = require('fs').promises;
        const path = require('path');
        
        try {
            console.log(chalk.yellow('🔄 Starting rollback process...'));
            
            const currentAppPath = path.join(__dirname, '..');
            
            // List of items to rollback
            const itemsToRollback = [
                { name: 'src', type: 'directory' },
                { name: 'config', type: 'directory' },
                { name: 'package.json', type: 'file' },
                { name: 'package-lock.json', type: 'file' }
            ];
            
            for (const item of itemsToRollback) {
                const backupItemPath = path.join(backup.path, item.name);
                const currentItemPath = path.join(currentAppPath, item.name);
                
                console.log(chalk.cyan(`🔄 Rolling back ${item.name}...`));
                
                // Check if backup item exists
                try {
                    await fs.access(backupItemPath);
                } catch {
                    console.log(chalk.yellow(`⚠️ Skipping ${item.name} - not found in backup`));
                    continue;
                }
                
                // Remove current item
                try {
                    await fs.rm(currentItemPath, { recursive: true, force: true });
                    console.log(chalk.green(`✅ Removed current ${item.name}`));
                } catch (removeError) {
                    console.log(chalk.yellow(`⚠️ Could not remove current ${item.name}: ${removeError.message}`));
                    // Continue anyway
                }
                
                // Copy from backup
                try {
                    if (item.type === 'directory') {
                        await this.copyDirectory(backupItemPath, currentItemPath);
                    } else {
                        await fs.copyFile(backupItemPath, currentItemPath);
                    }
                    console.log(chalk.green(`✅ Restored ${item.name} from backup`));
                } catch (copyError) {
                    throw new Error(`Failed to restore ${item.name}: ${copyError.message}`);
                }
            }
            
            // Update version in memory
            this.currentVersion = rollbackVersion;
            
            console.log(chalk.green(`✅ Rollback to version ${rollbackVersion} completed successfully`));
            
        } catch (error) {
            throw new Error(`Rollback process failed: ${error.message}`);
        }
    }

    /**
     * Restart application after rollback
     */
    async restartWithRollback(rollbackVersion) {
        const { spawn } = require('child_process');
        const path = require('path');

        try {
            console.log(chalk.magenta('🔄 Restarting application after rollback...'));

            // Create rollback restart script
            const rollbackScript = process.platform === 'win32' ? `
@echo off
echo ========================================
echo 🔄 AUTOMATIC ROLLBACK RESTART
echo ========================================
echo 📦 Rolled back to version: ${rollbackVersion}
echo 🕐 Time: %DATE% %TIME%
echo.

REM Change to application directory
cd /d "${path.join(__dirname, '..')}"
echo 📁 Application directory: %CD%

echo.
echo 📦 Installing dependencies for rolled back version...
call npm install --production 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ❌ NPM install failed - attempting to continue anyway...
    echo.
)

echo.
echo 🚀 Starting rolled back application...
echo ========================================
call npm start
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ❌ Application failed to start - trying direct node command...
    node src/vehicleConnector.js
)
echo.
echo ⚠️ Rollback restart completed - press any key to close
pause
            ` : `
#!/bin/bash
echo "========================================"
echo "🔄 AUTOMATIC ROLLBACK RESTART"
echo "========================================"
echo "📦 Rolled back to version: ${rollbackVersion}"
echo "🕐 Time: $(date)"
echo ""

# Change to application directory
cd "${path.join(__dirname, '..')}"
echo "📁 Application directory: $(pwd)"

echo ""
echo "📦 Installing dependencies for rolled back version..."
npm install --production
if [ $? -ne 0 ]; then
    echo ""
    echo "❌ NPM install failed - attempting to continue anyway..."
    echo ""
fi

echo ""
echo "🚀 Starting rolled back application..."
echo "========================================"
npm start
if [ $? -ne 0 ]; then
    echo ""
    echo "❌ Application failed to start - trying direct node command..."
    node src/vehicleConnector.js
fi
            `;

            const fs = require('fs').promises;
            const rollbackScriptPath = path.join(__dirname, '../rollback-restart.' + (process.platform === 'win32' ? 'bat' : 'sh'));

            await fs.writeFile(rollbackScriptPath, rollbackScript);

            // Set executable permissions on Unix-like systems
            if (process.platform !== 'win32') {
                await fs.chmod(rollbackScriptPath, '755');
            }

            console.log(chalk.magenta('🚀 Executing rollback restart script...'));

            const command = process.platform === 'win32' ? 'cmd' : 'bash';
            const args = process.platform === 'win32'
                ? ['/c', 'start', '"Rollback Restart"', '/wait', `"${rollbackScriptPath}"`]
                : [rollbackScriptPath];

            const child = spawn(command, args, {
                detached: true,
                stdio: 'ignore',
                cwd: path.join(__dirname, '..'),
                shell: true
            });

            child.unref();

            console.log(chalk.green('✅ Rollback restart script launched'));
            console.log(chalk.red('🔄 Current process will exit for rollback restart...'));

            setTimeout(() => {
                console.log(chalk.red('🔄 Rollback process completed - Exiting for restart...'));
                process.exit(0);
            }, 3000);

        } catch (error) {
            throw new Error(`Failed to restart after rollback: ${error.message}`);
        }
    }
}

module.exports = VehicleConnector;

// CLI execution
if (require.main === module) {
    const connector = new VehicleConnector();

    const gracefulShutdown = async (signal) => {
        console.log(chalk.yellow(`\n📡 Received ${signal} - Shutting down gracefully...`));
        await connector.disconnect();
        process.exit(0);
    };

    process.on('SIGINT', () => gracefulShutdown('SIGINT'));
    process.on('SIGTERM', () => gracefulShutdown('SIGTERM'));

    async function main() {
        try {
            await connector.initialize();
            await connector.connect();
        } catch (error) {
            console.error(chalk.red('❌ Application failed:'), error.message);
            process.exit(1);
        }
    }

    main();
}
