(define memories
  '((memory Banks (address (#x10000 . #x1fffff)) (type ANY))

	(memory Bank3 (address (#x30000 . #x3ffff)) (section heap) (type BSS))
	(block heap (size #xf400))
    (block stack (size #x0800))

    (memory Bank20 (address (#x200000 . #x20ffff)) (section farcode))
    (memory Bank21 (address (#x210000 . #x21ffff)) (section farcode))

	
    (memory LoMem (address (#x2100 . #xefff)) (type ANY))
     (memory vram (address (#xb00000 . #xefffff))
            (section vram))
    ))