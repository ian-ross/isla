import Data.Array
import Segmentation

test1 :: Grid Int
test1 = makeGrid (11,11) 0 // [(i,1) | i <- is1] // [(i,2) | i <- is2]
  where is1 = [(2,4), (2,7), (2,8)] ++
              concatMap (row [3..9]) [3..5] ++
              concatMap (row [3..5]) [6..9]
        is2 = row [7..8] 8 ++ row [7..9] 9
        row cs r = [(r,c) | c <- cs]
