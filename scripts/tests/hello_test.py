#coding=utf-8
import unittest

class TestHello(unittest.TestCase):
    # 该方法简单地返回字符串
    def say_hello(self):
        return "Hello World."
    # 计算两个整数的和
    def add(self, nA, nB):
        return nA + nB
    # 测试say_hello函数
    def test_a1_say_hello(self):
        self.assertEqual(self.say_hello() , "Hello World.")
    # 测试add函数
    def test_a2_add(self):
        self.assertEqual(self.add(3, 4) , 7)
        self.assertEqual(self.add(0, 4) , 4)
        self.assertEqual(self.add(-3, 0) , -3)