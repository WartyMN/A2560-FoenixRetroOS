(define memories
  '((memory Banks (address (#x10000 . #x1fffff)) (type ANY))

    (memory Bank20 (address (#x200000 . #x20ffff)) (section farcode))
    (memory Bank21 (address (#x210000 . #x21ffff)) (section farcode))

    (memory LoMem (address (#x2100 . #xefff)) (type ANY))
    (block stack (size #x0800))
    (block heap (size #xC500))
    (memory vram (address (#xb00000 . #xefffff))
            (section vram))
    ))