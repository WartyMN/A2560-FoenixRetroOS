(define memories
  '((memory Banks (address (#x10000 . #x1fffff)) (type ANY))

    (memory LoMem (address (#x2100 . #xefff)) (type ANY))

	(memory HeapStack (address (#x20000 . #x2ffff)) (section heap) (type BSS))
		(block heap (size #xf800))
    	(block stack (size #x0800))

    (memory FarCode1 (address (#x100000 . #x10ffff)) (section farcode))
    (memory FarCode2 (address (#x110000 . #x11ffff)) (section farcode))
    (memory FarCode3 (address (#x120000 . #x12ffff)) (section farcode))
	
     (memory VRAM (address (#xb00000 . #xefffff))
            (section vram))
    ))