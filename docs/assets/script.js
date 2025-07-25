// Main JavaScript for LAML Documentation
document.addEventListener('DOMContentLoaded', function() {
    // Mobile navigation toggle
    const navToggle = document.querySelector('.nav-toggle');
    const navLinks = document.querySelector('.nav-links');
    
    if (navToggle) {
        navToggle.addEventListener('click', function() {
            navLinks.classList.toggle('active');
            navToggle.classList.toggle('active');
        });
    }

    // Copy button functionality
    document.querySelectorAll('.copy-btn, .copy-code-btn').forEach(button => {
        button.addEventListener('click', function() {
            const targetElement = this.getAttribute('data-target') || 
                                  this.parentElement.querySelector('code') ||
                                  this.parentElement.querySelector('pre code') ||
                                  this.nextElementSibling;
            
            if (targetElement) {
                const textToCopy = targetElement.textContent || targetElement.innerText;
                copyToClipboard(textToCopy);
                showCopyFeedback(this);
            }
        });
    });

    // Demo playground functionality
    const demoBtn = document.querySelector('.demo-btn');
    if (demoBtn && demoBtn.textContent === 'Try in Playground') {
        demoBtn.addEventListener('click', function() {
            // Simulate running code in playground
            showNotification('Playground feature coming soon!', 'info');
        });
    }

    // Smooth scrolling for anchor links
    document.querySelectorAll('a[href^="#"]').forEach(anchor => {
        anchor.addEventListener('click', function (e) {
            e.preventDefault();
            const target = document.querySelector(this.getAttribute('href'));
            if (target) {
                target.scrollIntoView({
                    behavior: 'smooth',
                    block: 'start'
                });
            }
        });
    });

    // Auto-detect platform
    detectPlatform();

    // Search functionality (basic)
    initializeSearch();
});

// Platform detection
function detectPlatform() {
    const platformElement = document.getElementById('detected-platform');
    if (!platformElement) return;

    const userAgent = navigator.userAgent.toLowerCase();
    let platform = 'unknown';
    let icon = '❓';
    let name = 'Unknown Platform';
    let link = '#';

    if (userAgent.includes('win')) {
        platform = 'windows';
        icon = '🪟';
        name = 'Windows';
        link = '#windows';
    } else if (userAgent.includes('mac')) {
        platform = 'macos';
        icon = '🍎';
        name = 'macOS';
        link = '#linux'; // Use Linux installer for now
    } else if (userAgent.includes('linux')) {
        platform = 'linux';
        icon = '🐧';
        name = 'Linux';
        link = '#linux';
    } else if (userAgent.includes('android')) {
        platform = 'android';
        icon = '🤖';
        name = 'Android (Termux)';
        link = '#termux';
    }

    platformElement.innerHTML = `
        <div class="platform-icon">${icon}</div>
        <div class="platform-info">
            <h3>${name}</h3>
            <p>Recommended installer for your system</p>
        </div>
        <a href="${link}" class="btn btn-primary">Install for ${name}</a>
    `;
}

// Copy to clipboard functionality
function copyToClipboard(text) {
    if (navigator.clipboard && window.isSecureContext) {
        navigator.clipboard.writeText(text);
    } else {
        // Fallback for older browsers
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

// Show notification
function showNotification(message, type = 'info') {
    const notification = document.createElement('div');
    notification.className = `notification notification-${type}`;
    notification.textContent = message;
    
    // Style the notification
    notification.style.cssText = `
        position: fixed;
        top: 20px;
        right: 20px;
        background: ${type === 'info' ? '#3b82f6' : type === 'success' ? '#10b981' : '#ef4444'};
        color: white;
        padding: 1rem 1.5rem;
        border-radius: 0.5rem;
        box-shadow: 0 10px 15px -3px rgb(0 0 0 / 0.1);
        z-index: 9999;
        transform: translateX(100%);
        transition: transform 0.3s ease;
    `;
    
    document.body.appendChild(notification);
    
    // Animate in
    setTimeout(() => {
        notification.style.transform = 'translateX(0)';
    }, 10);
    
    // Remove after delay
    setTimeout(() => {
        notification.style.transform = 'translateX(100%)';
        setTimeout(() => {
            if (notification.parentNode) {
                notification.parentNode.removeChild(notification);
            }
        }, 300);
    }, 3000);
}

// Initialize search functionality
function initializeSearch() {
    const searchInput = document.querySelector('.search-input');
    if (!searchInput) return;

    let searchTimeout;
    searchInput.addEventListener('input', function() {
        clearTimeout(searchTimeout);
        searchTimeout = setTimeout(() => {
            performSearch(this.value);
        }, 300);
    });
}

// Basic search functionality
function performSearch(query) {
    if (query.length < 2) return;
    
    // This would typically connect to a search service
    // For now, we'll just show a placeholder
    console.log('Searching for:', query);
}

// Global function for command copying (referenced in HTML)
window.copyCommand = function() {
    const command = document.getElementById('install-command');
    if (command) {
        copyToClipboard(command.textContent);
        showCopyFeedback(event.target);
    }
};

// Keyboard shortcuts
document.addEventListener('keydown', function(e) {
    // Slash key to focus search
    if (e.key === '/' && !e.ctrlKey && !e.metaKey) {
        const searchInput = document.querySelector('.search-input');
        if (searchInput && document.activeElement !== searchInput) {
            e.preventDefault();
            searchInput.focus();
        }
    }
    
    // Escape to clear search
    if (e.key === 'Escape') {
        const searchInput = document.querySelector('.search-input');
        if (searchInput && document.activeElement === searchInput) {
            searchInput.value = '';
            searchInput.blur();
        }
    }
});
