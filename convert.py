#!/usr/bin/env python

import re
import sys

lines = []

with open(sys.argv[1]) as f:
  lines = f.readlines()

blocks      = []
block       = []
numBlankLines = 0

for line in lines:
  l = line.rstrip()

  if len(l) == 0:
    numBlankLines = numBlankLines + 1
    block.append(line)
    continue
  elif len(l) == 1:
    continue
  
  if re.match( r"[A-Z]{3,}.*", l ) or ( numBlankLines > 1 and len(l) > 10 and l[0].isupper() ):
    if block:
      blocks.append(block)
    block = [line]
  else:
    block.append(line)

  numBlankLines = 0

blocks.append(block)

for block in blocks:
  entry = "".join(block)
  entry = entry.replace('\n', '\\n')
  print(entry)
