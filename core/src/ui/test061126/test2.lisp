(load "C:/msys64/home/Tuy181201/.sbclrc")
(ql:quickload "cl-raylib" :silent t)

(cl-raylib:init-window 800 600 "Lisp + Interactive HTML UI")

(let ((ui-texture nil)
      (last-write-time 0)
      (ui-path "C:/Users/Tuy181201/Desktop/ui.png")
      (click-path "C:/Users/Tuy181201/Desktop/click.txt")
      (mouse-was-down nil)) ; <-- Переменная для защиты от спама кликами

  (loop while (not (cl-raylib:window-should-close))
        do 
           ;; 1. ПРОВЕРКА ОБНОВЛЕНИЙ
           (when (probe-file ui-path)
             (let ((current-write-time (ignore-errors (file-write-date ui-path))))
               (when (and current-write-time 
                          (or (null ui-texture) (> current-write-time last-write-time)))
                 (let ((new-tex (ignore-errors (cl-raylib:load-texture ui-path))))
                   (when new-tex
                     (when ui-texture (ignore-errors (cl-raylib:unload-texture ui-texture)))
                     (setf ui-texture new-tex)
                     (setf last-write-time current-write-time)
                     (format t "~&[LISP] UI updated from PNG!~%"))))))

           ;; 2. ОБРАБОТКА КЛИКА (Только один раз за нажатие)
           (let ((mouse-is-down (cl-raylib:is-mouse-button-pressed 0)))
             (when (and mouse-is-down (not mouse-was-down)) ; Проверяем, что кнопка ТОЛЬКО ЧТО нажата
               (let ((mx (cl-raylib:get-mouse-x))
                     (my (cl-raylib:get-mouse-y)))
                 (format t "~&[LISP] Mouse clicked at X:~A Y:~A~%" mx my)
                 (when (and (> mx 300) (< mx 500) (> my 250) (< my 300))
                   (format t "~&[LISP] HIT! Sending signal...~%")
                   (ignore-errors
                     (with-open-file (s click-path :direction :output :if-exists :supersede)
                       (format s "clicked"))))))
             (setf mouse-was-down mouse-is-down)) ; Запоминаем состояние для следующего кадра

           ;; 3. Отрисовка
           (cl-raylib:begin-drawing)
           (cl-raylib:clear-background (cl-raylib:make-rgba 30 30 30 255))
           (cl-raylib:draw-circle 400 300 50.0 (cl-raylib:make-rgba 255 0 0 255)) 
           
           (when ui-texture
             (cl-raylib:draw-texture ui-texture 0 0 (cl-raylib:make-rgba 255 255 255 255)))
           
           (cl-raylib:end-drawing))

  (when ui-texture (ignore-errors (cl-raylib:unload-texture ui-texture))))
(cl-raylib:close-window)