#coding=utf-8
import unittest
#from test_fk_math import TestFkMath
from hello_test import TestHello
from system_contract_tests.create_account_test import CreateAccount
# 从类中加载测试用例
suite1 = unittest.TestLoader().loadTestsFromTestCase( TestHello )
suite2 = unittest.TestLoader().loadTestsFromTestCase( CreateAccount )

s = [ suite1, suite2 ]
# 创建测试包
suite = unittest.TestSuite(s)

if __name__ == '__main__':
    # 创建测试运行器（TestRunner）
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run( suite )