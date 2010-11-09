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
import MosekMultiRbMultiFibers3D

cProfile.run('MosekMultiRbMultiFibers3D.GoProfile()', 'MosekMultiRbMultiFibers3D_profiled_result')

import pstats
p = pstats.Stats('MosekMultiRbMultiFibers3D_profiled_result')
p.sort_stats('cumulative').print_stats(10)
p.sort_stats('time').print_stats(10)
