#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Equation simplify check routine
"""

from sympy import Symbol, cos, sqrt, symbols, S, var
from sympy.simplify.cse_main import cse
from numpy import zeros, array
from scipy import sparse
import cPickle

basePath = '/media/vm/devel/aran/pymuscle/SymbolicEquations/'

dfdYKeysFile = open(basePath+'dfdYKeys.dat','r')
dfdYKeys = cPickle.load(dfdYKeysFile)
dfdYKeysFile.close()

reducedExprsFile = open(basePath+'reducedExprs.dat','r')
reducedExprs = cPickle.load(reducedExprsFile)
reducedExprsFile.close()

replacementsFile = open(basePath+'replacements.dat','r')
replacements = cPickle.load(replacementsFile)
replacementsFile.close()


simpFile = open(basePath+'simplified.txt', 'w')
for x, expression in replacements:
	simpFile.write('\t%s = %s\n' % (x, expression))


	
for i, expression in zip(range(15), reducedExprs[0:15]):
	simpFile.write('\tf[%d] = %s\n' % (i, expression))

assert len(reducedExprs) - 15 == len(dfdYKeys)
for expression, k in zip(reducedExprs[15:], dfdYKeys):
	simpFile.write('\tdfdY[%d,%d] = %s\n' % (k[0],k[1], expression))

simpFile.close()
