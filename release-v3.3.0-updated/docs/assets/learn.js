// JavaScript for the Learn page
document.addEventListener('DOMContentLoaded', function() {
    // Load the main script functionality
    if (window.location.pathname.includes('index.html') || window.location.pathname === '/') {
        return; // Main script handles index page
    }

    // Sidebar navigation toggle
    const sidebarToggle = document.querySelector('.sidebar-toggle');
    const sidebar = document.querySelector('.sidebar');
    
    if (sidebarToggle && sidebar) {
        sidebarToggle.addEventListener('click', function() {
            sidebar.classList.toggle('active');
            this.classList.toggle('active');
        });
    }

    // Collapsible sections in sidebar
    document.querySelectorAll('.section-header').forEach(header => {
        header.addEventListener('click', function() {
            const section = this.parentElement;
            const content = section.querySelector('.section-content');
            
            if (content) {
                section.classList.toggle('expanded');
                
                // Animate the content
                if (section.classList.contains('expanded')) {
                    content.style.maxHeight = content.scrollHeight + 'px';
                } else {
                    content.style.maxHeight = '0px';
                }
            }
        });
    });

    // Code example functionality
    initializeCodeExamples();

    // Progress tracking
    initializeProgressTracking();

    // Active navigation highlighting
    updateActiveNavigation();

    // Copy code functionality
    initializeCopyButtons();
});

// Initialize interactive code examples
function initializeCodeExamples() {
    document.querySelectorAll('.code-example').forEach((example, index) => {
        const codeBlock = example.querySelector('pre code');
        const runButton = example.querySelector('.run-code-btn');
        const output = example.querySelector('.code-output');
        
        if (runButton && codeBlock) {
            runButton.addEventListener('click', function() {
                runCodeExample(codeBlock.textContent, output, index);
            });
        }

        // Make code editable
        if (codeBlock) {
            makeCodeEditable(codeBlock);
        }
    });
}

// Make code blocks editable
function makeCodeEditable(codeBlock) {
    codeBlock.contentEditable = true;
    codeBlock.spellcheck = false;
    
    codeBlock.addEventListener('input', function() {
        // Simple syntax highlighting could be added here
        highlightSyntax(this);
    });

    codeBlock.addEventListener('keydown', function(e) {
        // Tab functionality
        if (e.key === 'Tab') {
            e.preventDefault();
            const selection = window.getSelection();
            const range = selection.getRangeAt(0);
            range.deleteContents();
            range.insertNode(document.createTextNode('    '));
            range.collapse();
            selection.removeAllRanges();
            selection.addRange(range);
        }
    });
}

// Basic syntax highlighting
function highlightSyntax(element) {
    let text = element.textContent;
    
    // Simple LAML syntax highlighting
    text = text
        .replace(/\b(let|const|if|else|for|while|function|return|import|export)\b/g, '<span class="keyword">$1</span>')
        .replace(/\b(\d+(\.\d+)?)\b/g, '<span class="number">$1</span>')
        .replace(/(["'])((?:(?!\1)[^\\]|\\.)*)(\1)/g, '<span class="string">$1$2$3</span>')
        .replace(/\/\/.*$/gm, '<span class="comment">$&</span>');
    
    element.innerHTML = text;
}

// Run code example (simulation)
function runCodeExample(code, outputElement, exampleIndex) {
    if (!outputElement) return;

    // Show loading state
    outputElement.innerHTML = '<div class="output-loading">Running...</div>';
    outputElement.style.display = 'block';

    // Simulate execution delay
    setTimeout(() => {
        let result = simulateCodeExecution(code, exampleIndex);
        outputElement.innerHTML = `<div class="output-result">${result}</div>`;
    }, 1000);
}

// Simulate code execution
function simulateCodeExecution(code, exampleIndex) {
    // Basic simulation based on code content
    if (code.includes('console.log') || code.includes('print')) {
        // Extract what's being printed
        const match = code.match(/(?:console\.log|print)\s*\(\s*["']([^"']+)["']\s*\)/);
        if (match) {
            return match[1];
        }
    }

    if (code.includes('let') || code.includes('const')) {
        return 'Variable declared successfully';
    }

    if (code.includes('function')) {
        return 'Function defined successfully';
    }

    if (code.includes('if')) {
        return 'Conditional logic executed';
    }

    // Default responses based on example index
    const defaultOutputs = [
        'Hello, LAML!',
        'Variable x = 42',
        'Function called successfully',
        'Array: [1, 2, 3, 4, 5]',
        'Loop completed',
        'Module imported successfully'
    ];

    return defaultOutputs[exampleIndex % defaultOutputs.length] || 'Code executed successfully';
}

// Initialize progress tracking
function initializeProgressTracking() {
    const progressBar = document.querySelector('.progress-bar');
    const progressText = document.querySelector('.progress-text');
    
    if (!progressBar) return;

    // Calculate progress based on completed sections
    updateProgress();

    // Mark sections as completed when viewed
    const observer = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                const section = entry.target;
                markSectionAsViewed(section);
                updateProgress();
            }
        });
    }, { threshold: 0.5 });

    // Observe all main content sections
    document.querySelectorAll('.content-section').forEach(section => {
        observer.observe(section);
    });
}

// Mark section as viewed
function markSectionAsViewed(section) {
    const sectionId = section.id;
    if (sectionId) {
        const viewedSections = getViewedSections();
        if (!viewedSections.includes(sectionId)) {
            viewedSections.push(sectionId);
            localStorage.setItem('laml-viewed-sections', JSON.stringify(viewedSections));
            
            // Update sidebar to show completion
            const navLink = document.querySelector(`a[href="#${sectionId}"]`);
            if (navLink) {
                navLink.classList.add('completed');
            }
        }
    }
}

// Get viewed sections from localStorage
function getViewedSections() {
    const stored = localStorage.getItem('laml-viewed-sections');
    return stored ? JSON.parse(stored) : [];
}

// Update progress bar
function updateProgress() {
    const progressBar = document.querySelector('.progress-fill');
    const progressText = document.querySelector('.progress-text');
    
    if (!progressBar) return;

    const totalSections = document.querySelectorAll('.content-section').length;
    const viewedSections = getViewedSections().length;
    const progress = totalSections > 0 ? (viewedSections / totalSections) * 100 : 0;

    progressBar.style.width = `${progress}%`;
    if (progressText) {
        progressText.textContent = `${Math.round(progress)}% Complete`;
    }
}

// Update active navigation
function updateActiveNavigation() {
    const currentHash = window.location.hash || '#getting-started';
    const navLinks = document.querySelectorAll('.nav-link');
    
    navLinks.forEach(link => {
        link.classList.remove('active');
        if (link.getAttribute('href') === currentHash) {
            link.classList.add('active');
        }
    });

    // Update on hash change
    window.addEventListener('hashchange', updateActiveNavigation);
}

// Initialize copy buttons for code blocks
function initializeCopyButtons() {
    document.querySelectorAll('pre').forEach(pre => {
        if (!pre.querySelector('.copy-code-btn')) {
            const copyBtn = document.createElement('button');
            copyBtn.className = 'copy-code-btn';
            copyBtn.innerHTML = '📋';
            copyBtn.title = 'Copy code';
            
            copyBtn.addEventListener('click', function() {
                const code = pre.querySelector('code');
                if (code) {
                    copyToClipboard(code.textContent);
                    showCopyFeedback(this);
                }
            });
            
            pre.style.position = 'relative';
            pre.appendChild(copyBtn);
        }
    });
}

// Copy to clipboard (reuse from main script)
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

// Show copy feedback (reuse from main script)
function showCopyFeedback(button) {
    const originalText = button.innerHTML;
    button.innerHTML = '✅';
    button.style.backgroundColor = '#10b981';
    
    setTimeout(() => {
        button.innerHTML = originalText;
        button.style.backgroundColor = '';
    }, 2000);
}

// Keyboard navigation
document.addEventListener('keydown', function(e) {
    // Arrow key navigation between sections
    if (e.altKey) {
        if (e.key === 'ArrowRight') {
            navigateToNextSection();
        } else if (e.key === 'ArrowLeft') {
            navigateToPreviousSection();
        }
    }
});

// Navigate to next section
function navigateToNextSection() {
    const currentHash = window.location.hash || '#getting-started';
    const navLinks = Array.from(document.querySelectorAll('.nav-link'));
    const currentIndex = navLinks.findIndex(link => link.getAttribute('href') === currentHash);
    
    if (currentIndex >= 0 && currentIndex < navLinks.length - 1) {
        window.location.hash = navLinks[currentIndex + 1].getAttribute('href');
    }
}

// Navigate to previous section
function navigateToPreviousSection() {
    const currentHash = window.location.hash || '#getting-started';
    const navLinks = Array.from(document.querySelectorAll('.nav-link'));
    const currentIndex = navLinks.findIndex(link => link.getAttribute('href') === currentHash);
    
    if (currentIndex > 0) {
        window.location.hash = navLinks[currentIndex - 1].getAttribute('href');
    }
}
