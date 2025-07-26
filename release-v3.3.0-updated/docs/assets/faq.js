// FAQ page JavaScript functionality
document.addEventListener('DOMContentLoaded', function() {
    // Initialize FAQ functionality
    initializeFAQItems();
    initializeSearch();
    initializeCategoryFilter();
    initializeInstallTabs();
    initializeCopyButtons();
});

// Initialize FAQ accordion items
function initializeFAQItems() {
    document.querySelectorAll('.faq-item').forEach(item => {
        const question = item.querySelector('.faq-question');
        const answer = item.querySelector('.faq-answer');
        
        question.addEventListener('click', function() {
            const isActive = item.classList.contains('active');
            
            // Close all other FAQ items
            document.querySelectorAll('.faq-item').forEach(otherItem => {
                if (otherItem !== item) {
                    otherItem.classList.remove('active');
                }
            });
            
            // Toggle current item
            if (isActive) {
                item.classList.remove('active');
            } else {
                item.classList.add('active');
                
                // Scroll into view if needed
                setTimeout(() => {
                    const rect = item.getBoundingClientRect();
                    if (rect.top < 100) {
                        item.scrollIntoView({ 
                            behavior: 'smooth', 
                            block: 'start' 
                        });
                    }
                }, 300);
            }
        });
    });
}

// Initialize search functionality
function initializeSearch() {
    const searchInput = document.getElementById('faq-search-input');
    if (!searchInput) return;

    let searchTimeout;
    searchInput.addEventListener('input', function() {
        clearTimeout(searchTimeout);
        searchTimeout = setTimeout(() => {
            performSearch(this.value.toLowerCase());
        }, 300);
    });

    // Search button
    const searchBtn = document.querySelector('.search-btn');
    if (searchBtn) {
        searchBtn.addEventListener('click', function() {
            performSearch(searchInput.value.toLowerCase());
        });
    }

    // Enter key search
    searchInput.addEventListener('keypress', function(e) {
        if (e.key === 'Enter') {
            performSearch(this.value.toLowerCase());
        }
    });
}

// Perform search in FAQ items
function performSearch(query) {
    const faqItems = document.querySelectorAll('.faq-item');
    const faqSections = document.querySelectorAll('.faq-section');
    const noResults = document.getElementById('no-results');
    let hasVisibleItems = false;

    // Clear category filter when searching
    if (query.trim()) {
        document.querySelectorAll('.topic-tag').forEach(tag => {
            tag.classList.remove('active');
        });
        document.querySelector('.topic-tag[data-category="all"]').classList.add('active');
    }

    faqItems.forEach(item => {
        const question = item.querySelector('.faq-question h3').textContent.toLowerCase();
        const answer = item.querySelector('.faq-answer').textContent.toLowerCase();
        
        const isMatch = !query.trim() || 
                       question.includes(query) || 
                       answer.includes(query);
        
        if (isMatch) {
            item.classList.remove('hidden');
            hasVisibleItems = true;
            
            // Highlight search terms
            if (query.trim()) {
                highlightSearchTerms(item, query);
            }
        } else {
            item.classList.add('hidden');
        }
    });

    // Show/hide sections based on visible items
    faqSections.forEach(section => {
        const visibleItems = section.querySelectorAll('.faq-item:not(.hidden)');
        if (visibleItems.length > 0) {
            section.classList.remove('hidden');
        } else {
            section.classList.add('hidden');
        }
    });

    // Show/hide no results message
    if (hasVisibleItems || !query.trim()) {
        noResults.style.display = 'none';
    } else {
        noResults.style.display = 'block';
    }
}

// Highlight search terms
function highlightSearchTerms(item, query) {
    const question = item.querySelector('.faq-question h3');
    const answer = item.querySelector('.faq-answer');
    
    // Remove existing highlights
    removeHighlights(item);
    
    if (!query.trim()) return;
    
    // Highlight in question
    highlightText(question, query);
    
    // Highlight in answer (but not in code blocks)
    const textNodes = getTextNodes(answer);
    textNodes.forEach(node => {
        if (!isInCodeBlock(node)) {
            highlightTextNode(node, query);
        }
    });
}

// Remove existing highlights
function removeHighlights(item) {
    item.querySelectorAll('.search-highlight').forEach(highlight => {
        const parent = highlight.parentNode;
        parent.replaceChild(document.createTextNode(highlight.textContent), highlight);
        parent.normalize();
    });
}

// Highlight text in element
function highlightText(element, query) {
    const text = element.textContent;
    const regex = new RegExp(`(${escapeRegex(query)})`, 'gi');
    const highlighted = text.replace(regex, '<span class="search-highlight">$1</span>');
    element.innerHTML = highlighted;
}

// Highlight text node
function highlightTextNode(node, query) {
    const text = node.textContent;
    const regex = new RegExp(`(${escapeRegex(query)})`, 'gi');
    
    if (regex.test(text)) {
        const highlighted = text.replace(regex, '<span class="search-highlight">$1</span>');
        const wrapper = document.createElement('div');
        wrapper.innerHTML = highlighted;
        
        const parent = node.parentNode;
        while (wrapper.firstChild) {
            parent.insertBefore(wrapper.firstChild, node);
        }
        parent.removeChild(node);
    }
}

// Get all text nodes
function getTextNodes(element) {
    const textNodes = [];
    const walker = document.createTreeWalker(
        element,
        NodeFilter.SHOW_TEXT,
        null,
        false
    );
    
    let node;
    while (node = walker.nextNode()) {
        textNodes.push(node);
    }
    
    return textNodes;
}

// Check if node is in code block
function isInCodeBlock(node) {
    let parent = node.parentElement;
    while (parent) {
        if (parent.tagName === 'CODE' || parent.tagName === 'PRE') {
            return true;
        }
        parent = parent.parentElement;
    }
    return false;
}

// Escape regex special characters
function escapeRegex(string) {
    return string.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
}

// Clear search
function clearSearch() {
    const searchInput = document.getElementById('faq-search-input');
    if (searchInput) {
        searchInput.value = '';
        performSearch('');
    }
}

// Initialize category filter
function initializeCategoryFilter() {
    document.querySelectorAll('.topic-tag').forEach(tag => {
        tag.addEventListener('click', function() {
            const category = this.getAttribute('data-category');
            filterByCategory(category);
            
            // Update active state
            document.querySelectorAll('.topic-tag').forEach(t => {
                t.classList.remove('active');
            });
            this.classList.add('active');
            
            // Clear search when filtering by category
            const searchInput = document.getElementById('faq-search-input');
            if (searchInput) {
                searchInput.value = '';
            }
        });
    });
}

// Filter FAQ items by category
function filterByCategory(category) {
    const faqItems = document.querySelectorAll('.faq-item');
    const faqSections = document.querySelectorAll('.faq-section');
    const noResults = document.getElementById('no-results');
    let hasVisibleItems = false;

    faqItems.forEach(item => {
        const itemCategory = item.getAttribute('data-category');
        
        if (category === 'all' || itemCategory === category) {
            item.classList.remove('hidden');
            hasVisibleItems = true;
        } else {
            item.classList.add('hidden');
        }
        
        // Remove search highlights when filtering
        removeHighlights(item);
    });

    // Show/hide sections
    faqSections.forEach(section => {
        const sectionCategory = section.getAttribute('data-category');
        const visibleItems = section.querySelectorAll('.faq-item:not(.hidden)');
        
        if (category === 'all' || sectionCategory === category || visibleItems.length > 0) {
            section.classList.remove('hidden');
        } else {
            section.classList.add('hidden');
        }
    });

    // Hide no results when filtering by category
    noResults.style.display = 'none';
}

// Initialize install tabs in FAQ answers
function initializeInstallTabs() {
    document.querySelectorAll('.install-tab').forEach(tab => {
        tab.addEventListener('click', function() {
            const platform = this.getAttribute('data-platform');
            const container = this.closest('.faq-answer');
            
            // Update tab states
            container.querySelectorAll('.install-tab').forEach(t => {
                t.classList.remove('active');
            });
            this.classList.add('active');
            
            // Update content states
            container.querySelectorAll('.install-content').forEach(content => {
                content.classList.remove('active');
            });
            
            const targetContent = container.querySelector(`#${platform}-install`);
            if (targetContent) {
                targetContent.classList.add('active');
            }
        });
    });
}

// Initialize copy buttons
function initializeCopyButtons() {
    document.querySelectorAll('.copy-btn, .copy-code-btn').forEach(button => {
        button.addEventListener('click', function() {
            const codeElement = this.parentElement.querySelector('code') || 
                               this.nextElementSibling?.querySelector('code') ||
                               this.closest('.code-example')?.querySelector('code') ||
                               this.closest('.command-block')?.querySelector('code');
            
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
    const originalText = button.textContent;
    button.textContent = 'Copied!';
    button.style.backgroundColor = '#10b981';
    button.style.color = 'white';
    
    setTimeout(() => {
        button.textContent = originalText;
        button.style.backgroundColor = '';
        button.style.color = '';
    }, 2000);
}

// Add CSS for search highlighting
const searchHighlightStyle = document.createElement('style');
searchHighlightStyle.textContent = `
    .search-highlight {
        background: linear-gradient(120deg, #fbbf24 0%, #f59e0b 100%);
        color: #1f2937;
        padding: 0.1em 0.2em;
        border-radius: 0.2em;
        font-weight: 600;
    }
`;
document.head.appendChild(searchHighlightStyle);

// Keyboard shortcuts
document.addEventListener('keydown', function(e) {
    // Ctrl/Cmd + K to focus search
    if ((e.ctrlKey || e.metaKey) && e.key === 'k') {
        e.preventDefault();
        const searchInput = document.getElementById('faq-search-input');
        if (searchInput) {
            searchInput.focus();
        }
    }
    
    // Escape to clear search
    if (e.key === 'Escape') {
        const searchInput = document.getElementById('faq-search-input');
        if (searchInput && document.activeElement === searchInput) {
            clearSearch();
            searchInput.blur();
        }
    }
});

// URL hash navigation for direct FAQ links
function handleHashNavigation() {
    const hash = window.location.hash;
    if (hash && hash.startsWith('#faq-')) {
        const faqId = hash.substring(1);
        const faqItem = document.getElementById(faqId);
        if (faqItem) {
            faqItem.classList.add('active');
            setTimeout(() => {
                faqItem.scrollIntoView({ 
                    behavior: 'smooth', 
                    block: 'start' 
                });
            }, 300);
        }
    }
}

// Handle hash navigation on load and hash change
window.addEventListener('load', handleHashNavigation);
window.addEventListener('hashchange', handleHashNavigation);
