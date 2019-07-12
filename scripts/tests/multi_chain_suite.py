#coding=utf-8
import unittest
#from test_fk_math import TestFkMath
from system_contract_tests.multi_chain_test import Multichain
# 从类中加载测试用例
MultiChainSuite = unittest.TestLoader().loadTestsFromTestCase( Multichain )
s = [
MultiChainSuite,
]
# 创建测试包
suite = unittest.TestSuite(s)

if __name__ == '__main__':
    # 创建测试运行器（TestRunner）
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run( suite )
