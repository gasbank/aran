#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

A generalized approach of simulating
rigid bodies and muscle fibers
with an implicit integration technique
"""
import cProfile
from box_lcp3_exp_multibody import GoTest

cProfile.run('GoTest()', 'profiled_result')

import pstats
p = pstats.Stats('profiled_result')
p.sort_stats('cumulative').print_stats(10)
p.sort_stats('time').print_stats(10)
