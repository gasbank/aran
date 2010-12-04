#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

A generalized approach of simulating
rigid bodies and muscle fibers
with an implicit integration technique
"""
import pstats
p = pstats.Stats('profiled_result')
p.sort_stats('cumulative').print_stats(20)
p.sort_stats('time').print_stats(20)
