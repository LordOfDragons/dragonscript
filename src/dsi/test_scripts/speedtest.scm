(define vRet 0)

(define (run times)
  (if (not (zero? times))
      (let ((vValue 1145677))
        (begin
          (set! vRet(/ (bitwise-and vValue 2047) 8.0))
          (set! vRet(/ (bitwise-and (arithmetic-shift vValue -11) 2047) 8.0))
          (set! vRet(/ (bitwise-and (arithmetic-shift vValue -22) 2047) 4.0))
          (run (- times 1))))))

(define (bench)
  (time (run 150000)))