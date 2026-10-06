/* Progressive enhancements only; all documents and navigation work without JS. */
document.querySelectorAll('[data-copy]').forEach(button => {
  button.addEventListener('click', async () => {
    const code = document.getElementById(button.dataset.copy);
    const value = code.textContent.replace(/\n$/, '');
    let copied = false;
    try {
      if (window.isSecureContext && navigator.clipboard) {
        await navigator.clipboard.writeText(value);
        copied = true;
      } else {
        const textarea = document.createElement('textarea');
        textarea.value = value;
        textarea.className = 'copy-fallback';
        document.body.appendChild(textarea);
        textarea.select();
        copied = document.execCommand('copy');
        textarea.remove();
      }
    } catch { copied = false; }
    if (!copied) {
      const range = document.createRange();
      range.selectNodeContents(code);
      window.getSelection().removeAllRanges();
      window.getSelection().addRange(range);
    }
    button.textContent = copied ? '已复制' : '请手动复制';
    document.getElementById('copy-status').textContent = copied ? '代码已复制到剪贴板' : '代码已选中，请手动复制';
    clearTimeout(button.copyTimer);
    button.copyTimer = setTimeout(() => { button.textContent = '复制'; }, 2200);
  });
});

document.querySelectorAll('.toc a, .inline-toc a').forEach(link => {
  link.addEventListener('click', () => {
    const target = document.getElementById(link.hash.slice(1));
    const details = target?.closest('details');
    if (details) details.open = true;
    const mobileToc = link.closest('.inline-toc');
    if (mobileToc) mobileToc.open = false;
  });
});

document.querySelectorAll('.mobile-menu nav a').forEach(link => {
  link.addEventListener('click', () => { link.closest('details').open = false; });
});

if ('IntersectionObserver' in window) {
  const observer = new IntersectionObserver(entries => {
    entries.forEach(entry => {
      if (!entry.isIntersecting) return;
      document.querySelectorAll('.toc a').forEach(link => {
        if (link.hash === '#' + entry.target.id) link.setAttribute('aria-current', 'location');
        else link.removeAttribute('aria-current');
      });
    });
  }, { rootMargin: '-12% 0px -70% 0px' });
  document.querySelectorAll('.article h2[id], .article summary[id]').forEach(heading => observer.observe(heading));
}
