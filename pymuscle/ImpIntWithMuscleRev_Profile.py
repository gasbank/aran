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
from ImpIntWithMuscleRev_Test import GoTest

cProfile.run('GoTest()', 'profiled_result')
