#coding=utf-8
import unittest

from hello_test import TestHello
from system_contract_tests.create_account import CreateAccount
from system_contract_tests.setglobalextendata import SetGlobalExtenData
from system_contract_tests.setprivupdatedparams import SetPrivUpdatedPrams
from system_contract_tests.setfreeaccount import SetFreeAccount
from system_contract_tests.buy_resource import BuyResourceLease
from system_contract_tests.regdelprod import RegDelProd
# 从类中加载测试用例
TestHelloSuite = unittest.TestLoader().loadTestsFromTestCase( TestHello )
CreateAccountSuite = unittest.TestLoader().loadTestsFromTestCase( CreateAccount )
SetGlobalExtenDataSuite = unittest.TestLoader().loadTestsFromTestCase( SetGlobalExtenData )
SetPrivUpdatedPramsSuite = unittest.TestLoader().loadTestsFromTestCase( SetPrivUpdatedPrams )
SetFreeAccountSuite = unittest.TestLoader().loadTestsFromTestCase( SetFreeAccount )
BuyResourceSuite = unittest.TestLoader().loadTestsFromTestCase( BuyResourceLease )
RegDelProdSuite = unittest.TestLoader().loadTestsFromTestCase( RegDelProd )
s = [
TestHelloSuite,
CreateAccountSuite,
SetGlobalExtenDataSuite,
SetPrivUpdatedPramsSuite,
SetFreeAccountSuite,
BuyResourceSuite,
RegDelProdSuite,
]
# 创建测试包
suite = unittest.TestSuite(s)

if __name__ == '__main__':
    # 创建测试运行器（TestRunner）
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run( suite )