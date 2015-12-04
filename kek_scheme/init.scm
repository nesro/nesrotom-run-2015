(define (null? a) (eq? a null))
(define (len a) (if (null? a) 0 (+ 1 (len (cdr a)))))
(define (append a b) (if (null? a) b (cons (car a) (append (cdr a) b))))
(define (fact_t n a) (if (= n 1) a (fact_t (- n 1) (* n a))))
(define (fact n) (fact_t n 1))
(define (applyrec fn arg next acc)
	(if (null? arg)
		acc
		(applyrec fn (next arg) next (fn acc arg))))

