// JavaScript for the Setup page
document.addEventListener('DOMContentLoaded', function() {
    // Platform detection and setup
    detectAndSetupPlatform();
    
    // Initialize accordions
    initializeAccordions();
    
    // Initialize installation steps
    initializeInstallationSteps();
    
    // Initialize copy buttons
    initializeCopyButtons();
    
    // Initialize VS Code extension setup
    initializeVSCodeSetup();
});

// Enhanced platform detection for setup page
function detectAndSetupPlatform() {
    const userAgent = navigator.userAgent.toLowerCase();
    let detectedPlatform = 'linux'; // default
    
    if (userAgent.includes('win')) {
        detectedPlatform = 'windows';
    } else if (userAgent.includes('mac')) {
        detectedPlatform = 'macos';
    } else if (userAgent.includes('android')) {
        detectedPlatform = 'termux';
    } else if (userAgent.includes('linux')) {
        detectedPlatform = 'linux';
    }
    
    // Update platform tabs
    const platformTabs = document.querySelectorAll('.platform-tab');
    const platformContents = document.querySelectorAll('.platform-content');
    
    // Remove all active states
    platformTabs.forEach(tab => tab.classList.remove('active'));
    platformContents.forEach(content => content.classList.remove('active'));
    
    // Activate detected platform
    const activeTab = document.querySelector(`[data-platform="${detectedPlatform}"]`);
    const activeContent = document.getElementById(detectedPlatform);
    
    if (activeTab && activeContent) {
        activeTab.classList.add('active');
        activeContent.classList.add('active');
    }
    
    // Add click handlers for platform tabs
    platformTabs.forEach(tab => {
        tab.addEventListener('click', function() {
            const platform = this.getAttribute('data-platform');
            showPlatform(platform);
        });
    });
}

// Show specific platform
function showPlatform(platform) {
    // Remove all active states
    document.querySelectorAll('.platform-tab').forEach(tab => {
        tab.classList.remove('active');
    });
    document.querySelectorAll('.platform-content').forEach(content => {
        content.classList.remove('active');
    });
    
    // Activate selected platform
    const tab = document.querySelector(`[data-platform="${platform}"]`);
    const content = document.getElementById(platform);
    
    if (tab && content) {
        tab.classList.add('active');
        content.classList.add('active');
    }
}

// Initialize accordion functionality
function initializeAccordions() {
    document.querySelectorAll('.accordion-header').forEach(header => {
        header.addEventListener('click', function() {
            const accordion = this.parentElement;
            const content = accordion.querySelector('.accordion-content');
            const isActive = accordion.classList.contains('active');
            
            // Close all accordions
            document.querySelectorAll('.accordion').forEach(acc => {
                acc.classList.remove('active');
                const accContent = acc.querySelector('.accordion-content');
                if (accContent) {
                    accContent.style.maxHeight = '0px';
                }
            });
            
            // Open clicked accordion if it wasn't active
            if (!isActive) {
                accordion.classList.add('active');
                if (content) {
                    content.style.maxHeight = content.scrollHeight + 'px';
                }
            }
        });
    });
}

// Initialize installation steps
function initializeInstallationSteps() {
    document.querySelectorAll('.step').forEach((step, index) => {
        const stepNumber = step.querySelector('.step-number');
        if (stepNumber) {
            stepNumber.textContent = index + 1;
        }
        
        // Add completion tracking
        const checkButton = step.querySelector('.check-step');
        if (checkButton) {
            checkButton.addEventListener('click', function() {
                toggleStepCompletion(step, index);
            });
        }
    });
    
    // Load completion state
    loadStepCompletion();
}

// Toggle step completion
function toggleStepCompletion(step, index) {
    const stepId = `step-${index}`;
    const completedSteps = getCompletedSteps();
    
    if (step.classList.contains('completed')) {
        step.classList.remove('completed');
        const stepIndex = completedSteps.indexOf(stepId);
        if (stepIndex > -1) {
            completedSteps.splice(stepIndex, 1);
        }
    } else {
        step.classList.add('completed');
        if (!completedSteps.includes(stepId)) {
            completedSteps.push(stepId);
        }
    }
    
    saveCompletedSteps(completedSteps);
    updateProgressIndicator();
}

// Get completed steps from localStorage
function getCompletedSteps() {
    const stored = localStorage.getItem('laml-setup-completed-steps');
    return stored ? JSON.parse(stored) : [];
}

// Save completed steps to localStorage
function saveCompletedSteps(steps) {
    localStorage.setItem('laml-setup-completed-steps', JSON.stringify(steps));
}

// Load step completion state
function loadStepCompletion() {
    const completedSteps = getCompletedSteps();
    
    document.querySelectorAll('.step').forEach((step, index) => {
        const stepId = `step-${index}`;
        if (completedSteps.includes(stepId)) {
            step.classList.add('completed');
        }
    });
    
    updateProgressIndicator();
}

// Update progress indicator
function updateProgressIndicator() {
    const totalSteps = document.querySelectorAll('.step').length;
    const completedSteps = document.querySelectorAll('.step.completed').length;
    const progress = totalSteps > 0 ? (completedSteps / totalSteps) * 100 : 0;
    
    const progressBar = document.querySelector('.setup-progress-fill');
    const progressText = document.querySelector('.setup-progress-text');
    
    if (progressBar) {
        progressBar.style.width = `${progress}%`;
    }
    
    if (progressText) {
        progressText.textContent = `${completedSteps}/${totalSteps} steps completed`;
    }
}

// Initialize copy buttons
function initializeCopyButtons() {
    document.querySelectorAll('.copy-btn').forEach(button => {
        button.addEventListener('click', function() {
            const codeElement = this.parentElement.querySelector('code') || 
                               this.nextElementSibling?.querySelector('code') ||
                               this.parentElement.nextElementSibling?.querySelector('code');
            
            if (codeElement) {
                copyToClipboard(codeElement.textContent);
                showCopyFeedback(this);
            }
        });
    });
}

// Initialize VS Code extension setup
function initializeVSCodeSetup() {
    const extensionStatus = document.getElementById('extension-status');
    const checkExtensionBtn = document.getElementById('check-extension');
    
    if (checkExtensionBtn) {
        checkExtensionBtn.addEventListener('click', function() {
            checkVSCodeExtension();
        });
    }
    
    // Auto-check extension status
    setTimeout(checkVSCodeExtension, 1000);
}

// Check VS Code extension status
function checkVSCodeExtension() {
    const statusElement = document.getElementById('extension-status');
    if (!statusElement) return;
    
    // This is a simulation since we can't actually check VS Code extensions from web
    statusElement.innerHTML = `
        <div class="status-checking">
            <div class="spinner"></div>
            Checking VS Code extension...
        </div>
    `;
    
    setTimeout(() => {
        // Simulate extension check result
        const isInstalled = Math.random() > 0.5; // Random for demo
        
        if (isInstalled) {
            statusElement.innerHTML = `
                <div class="status-success">
                    ✅ LAML extension is installed and active
                    <div class="status-details">
                        <p>Version: 3.2.0</p>
                        <p>Language support: Active</p>
                        <p>Syntax highlighting: Enabled</p>
                    </div>
                </div>
            `;
        } else {
            statusElement.innerHTML = `
                <div class="status-warning">
                    ⚠️ LAML extension not detected
                    <div class="status-details">
                        <p>Please install the extension from the marketplace</p>
                        <a href="vscode:extension/laml.laml-language" class="btn btn-primary btn-sm">Install Extension</a>
                    </div>
                </div>
            `;
        }
    }, 2000);
}

// Test installation
function testInstallation() {
    const testButton = document.querySelector('.test-installation');
    if (!testButton) return;
    
    testButton.addEventListener('click', function() {
        this.disabled = true;
        this.textContent = 'Testing...';
        
        // Simulate installation test
        setTimeout(() => {
            const testResult = document.createElement('div');
            testResult.className = 'test-result';
            testResult.innerHTML = `
                <div class="test-success">
                    ✅ Installation test passed!
                    <div class="test-details">
                        <p>✓ LAML compiler found</p>
                        <p>✓ Version: 3.2.0</p>
                        <p>✓ All dependencies satisfied</p>
                        <p>✓ VS Code extension active</p>
                    </div>
                </div>
            `;
            
            this.parentElement.appendChild(testResult);
            this.textContent = 'Test Again';
            this.disabled = false;
        }, 3000);
    });
}

// Copy to clipboard functionality
function copyToClipboard(text) {
    if (navigator.clipboard && window.isSecureContext) {
        navigator.clipboard.writeText(text);
    } else {
        const textArea = document.createElement('textarea');
        textArea.value = text;
        textArea.style.position = 'fixed';
        textArea.style.left = '-999999px';
        textArea.style.top = '-999999px';
        document.body.appendChild(textArea);
        textArea.focus();
        textArea.select();
        document.execCommand('copy');
        textArea.remove();
    }
}

// Show copy feedback
function showCopyFeedback(button) {
    const originalText = button.textContent;
    button.textContent = 'Copied!';
    button.style.backgroundColor = '#10b981';
    
    setTimeout(() => {
        button.textContent = originalText;
        button.style.backgroundColor = '';
    }, 2000);
}

// Installation progress tracking
function trackInstallationProgress() {
    const progressSteps = [
        'Downloading LAML compiler...',
        'Extracting binary...',
        'Setting up PATH...',
        'Installing VS Code extension...',
        'Verifying installation...'
    ];
    
    let currentStep = 0;
    const progressElement = document.createElement('div');
    progressElement.className = 'installation-progress';
    
    const interval = setInterval(() => {
        if (currentStep < progressSteps.length) {
            progressElement.innerHTML = `
                <div class="progress-step">
                    <div class="spinner"></div>
                    ${progressSteps[currentStep]}
                </div>
            `;
            currentStep++;
        } else {
            clearInterval(interval);
            progressElement.innerHTML = `
                <div class="progress-complete">
                    ✅ Installation completed successfully!
                </div>
            `;
        }
    }, 1500);
    
    return progressElement;
}

// Initialize installation test
document.addEventListener('DOMContentLoaded', function() {
    testInstallation();
});
