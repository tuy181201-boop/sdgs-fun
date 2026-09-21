/* ============================================================
   SDGS Documentation — Client-side logic
   Автор проекта: Куралесенко Дмитрий Владимирович
   Версия документа: 1.0
   ============================================================ */

(function () {
    'use strict';

    // ---------- Элементы ----------
    const sidebar   = document.getElementById('sidebar');
    const menuBtn   = document.getElementById('menuToggle');
    const toTopBtn  = document.getElementById('toTop');
    const navLinks  = document.querySelectorAll('.sidebar nav a');
    const sections  = document.querySelectorAll('section[id]');

    // ---------- Подсветка активного раздела при скролле ----------
    function updateActiveSection() {
        let current = '';
        sections.forEach(section => {
            const top = section.offsetTop - 120;
            if (window.scrollY >= top) current = section.id;
        });

        navLinks.forEach(link => {
            link.classList.toggle(
                'active',
                link.getAttribute('href') === '#' + current
            );
        });
    }

    // ---------- Показ/скрытие кнопки «вверх» ----------
    function updateToTopButton() {
        if (!toTopBtn) return;
        toTopBtn.style.display = window.scrollY > 400 ? 'block' : 'none';
    }

    // ---------- Плавная прокрутка по клику в меню ----------
    function setupSmoothScroll() {
        navLinks.forEach(link => {
            link.addEventListener('click', e => {
                e.preventDefault();
                const target = document.querySelector(link.getAttribute('href'));
                if (target) {
                    window.scrollTo({
                        top: target.offsetTop - 80,
                        behavior: 'smooth'
                    });
                    // Закрыть меню на мобильных
                    if (sidebar) sidebar.classList.remove('open');
                }
            });
        });
    }

    // ---------- Мобильное меню ----------
    function setupMobileMenu() {
        if (!menuBtn || !sidebar) return;
        menuBtn.addEventListener('click', () => {
            sidebar.classList.toggle('open');
        });
    }

    // ---------- Кнопка «вверх» ----------
    function setupToTopButton() {
        if (!toTopBtn) return;
        toTopBtn.addEventListener('click', () => {
            window.scrollTo({ top: 0, behavior: 'smooth' });
        });
    }

    // ---------- Единый обработчик скролла (с троттлингом) ----------
    function setupScrollHandler() {
        let ticking = false;
        window.addEventListener('scroll', () => {
            if (!ticking) {
                window.requestAnimationFrame(() => {
                    updateActiveSection();
                    updateToTopButton();
                    ticking = false;
                });
                ticking = true;
            }
        });
    }

    // ---------- Инициализация ----------
    function init() {
        console.log('%c🌍 SDGS Documentation', 'color: #00d4ff; font-size: 20px; font-weight: bold;');
        console.log('%cПолная техническая документация загружена.', 'color: #8899bb;');
        console.log('%cАвтор: Куралесенко Д. В.', 'color: #aa88ff;');

        setupSmoothScroll();
        setupMobileMenu();
        setupToTopButton();
        setupScrollHandler();

        // Первичная синхронизация
        updateActiveSection();
        updateToTopButton();
    }

    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
})();