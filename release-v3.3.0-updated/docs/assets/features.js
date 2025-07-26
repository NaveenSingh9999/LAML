// Features page JavaScript functionality
document.addEventListener('DOMContentLoaded', function() {
    // Initialize all features functionality
    initializeDemoTabs();
    initializeCategoryFilter();
    initializeFeatureCards();
    initializeBenchmarkAnimations();
    initializeCopyButtons();
});

// Initialize demo tabs in hero section
function initializeDemoTabs() {
    const demoTabs = document.querySelectorAll('.demo-tab');
    const demoPanels = document.querySelectorAll('.demo-panel');
    
    demoTabs.forEach(tab => {
        tab.addEventListener('click', function() {
            const targetDemo = this.getAttribute('data-demo');
            
            // Update tab states
            demoTabs.forEach(t => t.classList.remove('active'));
            this.classList.add('active');
            
            // Update panel states
            demoPanels.forEach(panel => {
                panel.classList.remove('active');
            });
            
            const targetPanel = document.getElementById(`${targetDemo}-demo`);
            if (targetPanel) {
                targetPanel.classList.add('active');
            }
        });
    });
}

// Initialize category filtering
function initializeCategoryFilter() {
    const categoryBtns = document.querySelectorAll('.category-btn');
    const featureCards = document.querySelectorAll('.feature-card');
    
    categoryBtns.forEach(btn => {
        btn.addEventListener('click', function() {
            const category = this.getAttribute('data-category');
            
            // Update button states
            categoryBtns.forEach(b => b.classList.remove('active'));
            this.classList.add('active');
            
            // Filter feature cards
            filterFeatureCards(category, featureCards);
        });
    });
}

// Filter feature cards by category
function filterFeatureCards(category, featureCards) {
    featureCards.forEach((card, index) => {
        const cardCategory = card.getAttribute('data-category');
        const shouldShow = category === 'all' || cardCategory === category;
        
        if (shouldShow) {
            // Staggered animation for showing cards
            setTimeout(() => {
                card.classList.remove('hidden');
            }, index * 50);
        } else {
            card.classList.add('hidden');
        }
    });
}

// Initialize feature card interactions
function initializeFeatureCards() {
    const featureCards = document.querySelectorAll('.feature-card');
    
    // Add intersection observer for scroll animations
    const observer = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                entry.target.style.opacity = '1';
                entry.target.style.transform = 'translateY(0)';
            }
        });
    }, {
        threshold: 0.1,
        rootMargin: '0px 0px -50px 0px'
    });
    
    featureCards.forEach(card => {
        // Initially hide cards for animation
        card.style.opacity = '0';
        card.style.transform = 'translateY(20px)';
        card.style.transition = 'opacity 0.6s ease, transform 0.6s ease';
        
        // Observe for scroll animation
        observer.observe(card);
        
        // Add hover effects for code examples
        const codeExample = card.querySelector('.feature-example');
        if (codeExample) {
            addCodeExampleInteraction(codeExample);
        }
    });
}

// Add interaction to code examples
function addCodeExampleInteraction(codeExample) {
    const pre = codeExample.querySelector('pre');
    if (!pre) return;
    
    // Add copy button
    const copyBtn = document.createElement('button');
    copyBtn.className = 'copy-example-btn';
    copyBtn.innerHTML = '📋';
    copyBtn.title = 'Copy code';
    
    copyBtn.addEventListener('click', function() {
        const code = pre.querySelector('code');
        if (code) {
            copyToClipboard(code.textContent);
            showCopyFeedback(this);
        }
    });
    
    codeExample.style.position = 'relative';
    codeExample.appendChild(copyBtn);
    
    // Add syntax highlighting effect
    const code = pre.querySelector('code');
    if (code) {
        applySyntaxHighlighting(code);
    }
}

// Apply basic syntax highlighting
function applySyntaxHighlighting(codeElement) {
    let html = codeElement.innerHTML;
    
    // LAML syntax highlighting
    html = html
        .replace(/\b(let|const|function|if|else|for|while|return|import|export|async|await|class|new|try|catch|finally|throw)\b/g, 
                '<span class="keyword">$1</span>')
        .replace(/\b(\d+(?:\.\d+)?)\b/g, '<span class="number">$1</span>')
        .replace(/(["'])((?:(?!\1)[^\\]|\\.)*)(\1)/g, '<span class="string">$1$2$3</span>')
        .replace(/\/\/.*$/gm, '<span class="comment">$&</span>')
        .replace(/\b(true|false|null|undefined)\b/g, '<span class="literal">$1</span>');
    
    codeElement.innerHTML = html;
}

// Initialize benchmark animations
function initializeBenchmarkAnimations() {
    const benchmarkFills = document.querySelectorAll('.benchmark-fill');
    
    const observer = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                animateBenchmarkBar(entry.target);
            }
        });
    }, {
        threshold: 0.5
    });
    
    benchmarkFills.forEach(fill => {
        // Store original width
        const originalWidth = fill.style.width;
        fill.dataset.targetWidth = originalWidth;
        fill.style.width = '0%';
        
        observer.observe(fill);
    });
}

// Animate benchmark bar
function animateBenchmarkBar(fillElement) {
    const targetWidth = fillElement.dataset.targetWidth;
    const duration = 1500;
    const startTime = performance.now();
    
    function animate(currentTime) {
        const elapsed = currentTime - startTime;
        const progress = Math.min(elapsed / duration, 1);
        
        // Easing function for smooth animation
        const easedProgress = easeOutCubic(progress);
        const currentWidth = parseFloat(targetWidth) * easedProgress;
        
        fillElement.style.width = `${currentWidth}%`;
        
        if (progress < 1) {
            requestAnimationFrame(animate);
        }
    }
    
    requestAnimationFrame(animate);
}

// Easing function
function easeOutCubic(t) {
    return 1 - Math.pow(1 - t, 3);
}

// Initialize copy buttons
function initializeCopyButtons() {
    document.querySelectorAll('.copy-example-btn').forEach(button => {
        button.addEventListener('click', function() {
            const codeElement = this.parentElement.querySelector('code');
            if (codeElement) {
                copyToClipboard(codeElement.textContent);
                showCopyFeedback(this);
            }
        });
    });
}

// Copy to clipboard
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
    const originalText = button.innerHTML;
    button.innerHTML = '✅';
    button.style.backgroundColor = '#10b981';
    
    setTimeout(() => {
        button.innerHTML = originalText;
        button.style.backgroundColor = '';
    }, 2000);
}

// Search functionality for features
function initializeFeatureSearch() {
    const searchInput = document.querySelector('.search-input');
    if (!searchInput) return;
    
    let searchTimeout;
    searchInput.addEventListener('input', function() {
        clearTimeout(searchTimeout);
        searchTimeout = setTimeout(() => {
            performFeatureSearch(this.value.toLowerCase());
        }, 300);
    });
}

// Perform feature search
function performFeatureSearch(query) {
    const featureCards = document.querySelectorAll('.feature-card');
    
    featureCards.forEach(card => {
        const title = card.querySelector('h3').textContent.toLowerCase();
        const description = card.querySelector('p').textContent.toLowerCase();
        const codeExample = card.querySelector('.feature-example code');
        const codeText = codeExample ? codeExample.textContent.toLowerCase() : '';
        
        const isMatch = !query.trim() || 
                       title.includes(query) || 
                       description.includes(query) ||
                       codeText.includes(query);
        
        if (isMatch) {
            card.classList.remove('hidden');
        } else {
            card.classList.add('hidden');
        }
    });
}

// Add CSS for copy buttons and syntax highlighting
const featureStyles = document.createElement('style');
featureStyles.textContent = `
    .copy-example-btn {
        position: absolute;
        top: 0.75rem;
        right: 0.75rem;
        background: var(--secondary);
        border: 1px solid var(--border);
        border-radius: 0.25rem;
        padding: 0.25rem 0.5rem;
        font-size: 0.8rem;
        cursor: pointer;
        transition: all 0.2s ease;
        opacity: 0.7;
    }
    
    .copy-example-btn:hover {
        background: var(--hover);
        opacity: 1;
    }
    
    .feature-example:hover .copy-example-btn {
        opacity: 1;
    }
    
    /* Syntax highlighting */
    .keyword {
        color: #8b5cf6;
        font-weight: 600;
    }
    
    .number {
        color: #f97316;
    }
    
    .string {
        color: #10b981;
    }
    
    .comment {
        color: #6b7280;
        font-style: italic;
    }
    
    .literal {
        color: #ef4444;
        font-weight: 500;
    }
`;
document.head.appendChild(featureStyles);

// Performance monitoring
function trackFeatureInteractions() {
    // Track which features users interact with most
    document.querySelectorAll('.feature-card').forEach((card, index) => {
        card.addEventListener('click', function() {
            const featureName = this.querySelector('h3').textContent;
            console.log(`Feature clicked: ${featureName}`);
            
            // You could send this data to analytics
            // analytics.track('feature_clicked', { feature: featureName });
        });
    });
    
    // Track demo tab interactions
    document.querySelectorAll('.demo-tab').forEach(tab => {
        tab.addEventListener('click', function() {
            const demoType = this.getAttribute('data-demo');
            console.log(`Demo tab clicked: ${demoType}`);
            
            // analytics.track('demo_tab_clicked', { demo: demoType });
        });
    });
}

// Initialize performance tracking
trackFeatureInteractions();

// Keyboard shortcuts
document.addEventListener('keydown', function(e) {
    // Number keys 1-6 to switch categories
    if (e.key >= '1' && e.key <= '6') {
        const categoryBtns = document.querySelectorAll('.category-btn');
        const index = parseInt(e.key) - 1;
        
        if (categoryBtns[index]) {
            categoryBtns[index].click();
        }
    }
    
    // 'D' key to switch demo tabs
    if (e.key.toLowerCase() === 'd') {
        const activeDemoTab = document.querySelector('.demo-tab.active');
        const allDemoTabs = document.querySelectorAll('.demo-tab');
        const currentIndex = Array.from(allDemoTabs).indexOf(activeDemoTab);
        const nextIndex = (currentIndex + 1) % allDemoTabs.length;
        
        allDemoTabs[nextIndex].click();
    }
});

// Auto-rotate demo tabs for engagement
function autoRotateDemoTabs() {
    const demoTabs = document.querySelectorAll('.demo-tab');
    let currentIndex = 0;
    
    setInterval(() => {
        // Only auto-rotate if user hasn't interacted recently
        const timeSinceLastInteraction = Date.now() - (window.lastDemoInteraction || 0);
        
        if (timeSinceLastInteraction > 10000) { // 10 seconds
            currentIndex = (currentIndex + 1) % demoTabs.length;
            demoTabs[currentIndex].click();
        }
    }, 5000); // 5 seconds
}

// Track user interactions to pause auto-rotation
document.querySelectorAll('.demo-tab').forEach(tab => {
    tab.addEventListener('click', function() {
        window.lastDemoInteraction = Date.now();
    });
});

// Start auto-rotation
autoRotateDemoTabs();
