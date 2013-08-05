# -*- coding: UTF-8 -*-

import wx
import os
import sys, platform

os_info = platform.system()
if cmp(os_info, "Linux"):
	from skyeye_common_windows_module import *
	import windows_font as C
else:
	from skyeye_common_module import *
	import linux_font as C

class InfoRegsDialog(wx.Dialog):
	def __init__(self):
		wx.Dialog.__init__(self, None, -1, C.FontInfoRegs ,size=(500, 500),
			style=wx.DEFAULT_DIALOG_STYLE|wx.MAXIMIZE_BOX|wx.MINIMIZE_BOX)
		self.RegsList = wx.ListCtrl(self, -1, style = wx.LC_REPORT | wx.LC_HRULES)
		self.RegsList.InsertColumn(0, C.FontReg, format = wx.LIST_FORMAT_LEFT, width = 100)
		self.RegsList.InsertColumn(1, C.FontHex, format = wx.LIST_FORMAT_LEFT, width = 180)
		self.RegsList.InsertColumn(2, C.FontDec, format = wx.LIST_FORMAT_LEFT, width = 220)
		# Refurbish the regs' informations
		self.RegsRefurbish()

	def GetRegsInfo(self):
		print "get a struct that includes reg's name and value."

	def ShowRegsInfo(self):
		print "Show the struct on list."

	def RegsRefurbish(self):
		print "Refurbish when stop skyeye."

if __name__ == '__main__':
	app = wx.PySimpleApp()
	Regsdlg = InfoRegsDialog()
	Regsdlg.Show()
	app.MainLoop()
