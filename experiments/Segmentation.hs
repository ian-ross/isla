{-# LANGUAGE FlexibleInstances #-}

module Segmentation
       ( Grid, makeGrid, putGrid, putGridAndBoxes, putMask
       , LMass, Box, Wise(..)
       , findBoxes, segment, greedy, scored
       ) where

import Data.Array
import Data.List
import Data.Function

----------------------------------------------------------------------
--
--  GRIDS
--

type Grid a = Array (Int,Int) a

makeGrid :: (Int,Int) -> a -> Grid a
makeGrid (w,h) v = listArray ((1,1), (w,h)) $ repeat v

putGrid :: Show a => Grid a -> IO ()
putGrid g = do
  putStrLn $ intercalate "\n" [printrow r | r <- [1..h]]
  where (_, (w, h)) = bounds g
        maxlen = 1 + (maximum $ map (length . show) $ elems g)
        row r = [show $ g ! (r, c) | c <- [1..w]]
        padrow = map (\s -> replicate (maxlen - length s) ' ' ++ s) . row
        printrow = concat . padrow

putGridWith :: (a -> String) -> Grid a -> IO ()
putGridWith shower g = do
  putStrLn $ intercalate "\n" [printrow r | r <- [1..h]]
  where (_, (w, h)) = bounds g
        maxlen = 1 + (maximum $ map (length . shower) $ elems g)
        row r = [shower $ g ! (r, c) | c <- [1..w]]
        padrow = map (\s -> replicate (maxlen - length s) ' ' ++ s) . row
        printrow = concat . padrow

putGridAndBoxes :: Show a => Grid a -> [Box] -> IO ()
putGridAndBoxes g bs = do
  putStrLn $ intercalate "\n" [printrow r | r <- [1..h]]
  where (_, (w, h)) = bounds g
        maxlen = 3 + (maximum $ map (length . show) $ elems g)
        row r = [wrap (r, c) (show $ g ! (r, c)) | c <- [1..w]]
        padrow = map (\s -> replicate (maxlen - length s) ' ' ++ s) . row
        printrow = concat . padrow
        wrap (r, c) s
          | any ((r,c) `inBox`) bs = "[" ++ s ++ "]"
          | otherwise = " " ++ s ++ " "

inBox :: (Int,Int) -> Box -> Bool
inBox (r,c) (Box l b w h) = c >= l && c < l + w && r >= b && r < b + h

boolShower :: Bool -> String
boolShower True = "X"
boolShower False = "."

putMask :: Grid Bool -> IO ()
putMask = putGridWith boolShower


----------------------------------------------------------------------
--
--  GEOMETRICAL TYPES
--

type LMass = Int

-- (row, column)
data Cell = Cell Int Int deriving (Eq, Show)

-- (left, bottom, width, height)
data Box = Box Int Int Int Int deriving (Eq, Show)


----------------------------------------------------------------------
--
--  BOUNDING BOXES, ROWS AND COLUMNS
--

boundBox :: Grid LMass -> LMass -> Maybe Box
boundBox g lm = case lmcells of
  [] -> Nothing
  _ -> Just $ Box minc minr (maxc-minc+1) (maxr-minr+1)
  where minc = minimum cs ; maxc = maximum cs
        minr = minimum rs ; maxr = maximum rs
        lmcells = map fst . filter (\(_,v) -> v == lm) . assocs $ g
        rs = map fst lmcells ; cs = map snd lmcells

boundRows :: Grid LMass -> LMass -> [Box]
boundRows g lm = case boundBox g lm of
  Nothing -> []
  Just (Box _ bbb _ bbh) -> concatMap makeBoxes [minr..maxr]
    where minr = bbb ; maxr = bbb + bbh - 1
          lmcells = map fst . filter (\(_,v) -> v == lm) . assocs $ g
          makeBoxes br = map (\(s,e) -> Box s br (e-s+1) 1) .
                         runs . map snd . filter ((==br) . fst) $ lmcells

boundCols :: Grid LMass -> LMass -> [Box]
boundCols g lm = case boundBox g lm of
  Nothing -> []
  Just (Box bbl _ bbw _) -> concatMap makeBoxes [minc..maxc]
    where minc = bbl ; maxc = bbl + bbw - 1
          lmcells = map fst . filter (\(_,v) -> v == lm) . assocs $ g
          makeBoxes bc = map (\(s,e) -> Box bc s 1 (e-s+1)) .
                         runs . map fst . filter ((==bc) . snd) $ lmcells


----------------------------------------------------------------------
--
--  ADMISSIBILITY OF BOXES AND COLLECTIONS OF BOXES
--

class Admissible a where
  admissible :: Grid Bool -> a -> Bool

instance Admissible Cell where
  admissible g (Cell r c) = g ! (r, c)

instance Admissible Box where
  admissible g (Box l b w h) = all (admissible g) cells
    where cells = [Cell r c | r <- [b..b+h-1], c <- [l..l+w-1]]

instance Admissible [Box] where
  admissible g bs = all (admissible g) bs && not (overlap bs)

overlap :: [Box] -> Bool
overlap bs = any (>=2) $ elems chk
  where chk = foldl' addRefs chk0 bs
        chk0 = listArray ((1,1), (gw,gh)) $ repeat (0 :: Int)
        gw = maximum $ map (\(Box l _ w _) -> l+w-1) bs
        gh = maximum $ map (\(Box _ b _ h) -> b+h-1) bs
        addRefs a (Box l b w h) = a // [((r,c), a ! (r,c) + 1) |
                                        r <- [l..l+w-1], c <- [b..b+h-1]]

----------------------------------------------------------------------
--
--  BOX MERGING
--

type Method = Grid LMass -> LMass -> Grid Bool -> [Box] -> Maybe [Box]

merge :: Box -> Box -> Box
merge (Box l1 b1 w1 h1) (Box l2 b2 w2 h2) = Box l b w h
  where l = l1 `min` l2 ; b = b1 `min` b2
        w = r -l + 1 ; h = t - b + 1
        r = (l1+w1-1) `max` (l2+w2-1) ; t = (b1+h1-1) `max` (b2+h2-1)

greedyStep :: Grid Bool -> [Box] -> Maybe [Box]
greedyStep _ [] = Just []
greedyStep g bs
  | length bs == 1 = Just bs
  | null okmerges = Nothing
  | otherwise = Just newbs
    where cands = [(i,j) | i <- [0..length bs-1], j <- [i+1..length bs-1]]
          merges = map (\(i,j) -> ((i,j), (bs !! i) `merge` (bs !! j))) cands
          okmerges = filter (\(_,b) -> admissible g b) merges
          ((mergei, mergej), mergebox) = head okmerges
          newbs = mergebox : deleteAt mergei (deleteAt mergej bs)

greedy :: Method
greedy _ _ _ [] = Just []
greedy glm lm g bs
  | not (admissible g bs) = Nothing
  | length bs == 1 = Just bs
  | otherwise = case greedyStep g bs of
    Nothing -> Just bs
    Just bs' -> greedy glm lm g bs'

score :: Grid LMass -> LMass -> Grid Bool -> [Box] -> Int
score glm lm gad bs
  | not (admissible gad bs) = gw * gh + 1
  | otherwise = sum $ map count $ bs
    where (_, (gw,gh)) = bounds glm
          count (Box l b w h) =
            sum $ map (\i -> if glm ! i /= lm then 1 else 0) bi
            where bi = [(r,c) | r <- [b..b+h-1], c <- [l..l+w-1]]

scoredStep :: Grid LMass -> LMass -> Grid Bool -> [Box] -> Maybe [Box]
scoredStep _ _ _ [] = Just []
scoredStep glm lm g bs
  | length bs == 1 = Just bs
  | null okmerges = Nothing
  | otherwise = Just newbs
    where cands = [(i,j) | i <- [0..length bs-1], j <- [i+1..length bs-1]]
          merges = map (\(i,j) -> ((i,j), (bs !! i) `merge` (bs !! j))) cands
          okmerges = filter (\(_,b) -> admissible g b) merges
          scoredmerges = map scoreMerge okmerges
          (_, ((mi, mj), mbox)) = minimumBy (compare `on` fst) scoredmerges
          newbs = mbox : deleteAt mi (deleteAt mj bs)
          scoreMerge m@((i, j), b) = (score glm lm g bs', m)
            where bs' = b : deleteAt i (deleteAt j bs)

scored :: Method
scored _ _ _ [] = Just []
scored glm lm g bs
  | not (admissible g bs) = Nothing
  | length bs == 1 = Just bs
  | otherwise = case scoredStep glm lm g bs of
    Nothing -> Just bs
    Just bs' -> scored glm lm g bs'


----------------------------------------------------------------------
--
--  TOP-LEVEL FUNCTIONS
--

data Wise = Cols | Rows

findBoxes :: Method -> Wise -> Grid LMass -> LMass -> Maybe [Box]
findBoxes meth w glm lm = meth glm lm adm (wfn glm lm)
  where wfn = case w of
          Cols -> boundCols
          Rows -> boundRows
        adm = fmap (\x -> x==lm || x==0) glm

segment :: Grid LMass -> LMass -> Maybe [Box]
segment glm lm =
  case (byrows, bycols) of
    (Nothing, Nothing) -> Nothing
    (Nothing, Just bs) -> Just bs
    (Just bs, Nothing) -> Just bs
    (Just bs1, Just bs2) ->
      if length bs1 <= length bs2 then Just bs1 else Just bs2
  where byrows = findBoxes scored Rows glm lm
        bycols = findBoxes scored Cols glm lm

----------------------------------------------------------------------
--
--  UTILITIES
--

runs :: [Int] -> [(Int,Int)]
runs [] = []
runs is = reverse $ go (head is) (head is) (tail is) []
  where go s cur [] acc = (s,cur) : acc
        go s cur (x:xs) acc
          | x == cur + 1 = go s (cur+1) xs acc
          | otherwise = go x x xs ((s,cur) : acc)

deleteAt :: Int -> [a] -> [a]
deleteAt _ [] = []
deleteAt 0 (_:xs) = xs
deleteAt n (x:xs) = x : deleteAt (n-1) xs
