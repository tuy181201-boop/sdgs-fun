/* SDGS Documentation — subpage helpers
   Автор проекта: Куралесенко Дмитрий Владимирович */
(function () {
    'use strict';
    const sidebar  = document.getElementById('sidebar');
    const menuBtn  = document.getElementById('menuToggle');
    const toTopBtn = document.getElementById('toTop');

    if (menuBtn && sidebar) {
        menuBtn.addEventListener('click', () => {
            const isOpen = sidebar.classList.toggle('open');
            menuBtn.setAttribute('aria-expanded', String(isOpen));
        });
    }

    if (toTopBtn) {
        const updateTop = () => {
            toTopBtn.style.display = window.scrollY > 400 ? 'block' : 'none';
        };
        toTopBtn.addEventListener('click', () => {
            window.scrollTo({ top: 0, behavior: 'smooth' });
        });
        window.addEventListener('scroll', updateTop, { passive: true });
        updateTop();
    }
})();
