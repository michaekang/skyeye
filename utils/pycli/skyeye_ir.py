# -*- coding: UTF-8 -*-

import wx
import os
import sys, platform
from ctypes import *
import string

os_info = platform.system()
if cmp(os_info, "Linux"):
	from skyeye_common_windows_module import *
	import windows_font as C
else:
	from skyeye_common_module import *
	import linux_font as C

class InfoRegsDialog(wx.Dialog):
	def __init__(self):
		wx.Dialog.__init__(self, None, -1, C.FontInfoRegs ,size=(500, 410),
			style=wx.DEFAULT_DIALOG_STYLE|wx.MAXIMIZE_BOX|wx.MINIMIZE_BOX)

		sizer = wx.GridBagSizer(hgap=1, vgap=1)
		self.RegsList = wx.ListCtrl(self, -1, size = (500, 400), style = wx.LC_REPORT | wx.LC_HRULES)
		self.RegsList.InsertColumn(0, C.FontReg, format = wx.LIST_FORMAT_LEFT, width = 100)
		self.RegsList.InsertColumn(1, C.FontHex, format = wx.LIST_FORMAT_LEFT, width = 180)
		self.RegsList.InsertColumn(2, C.FontDec, format = wx.LIST_FORMAT_LEFT, width = 200)
		# Refurbish the regs' informations
		sizer.Add(self.RegsList, pos = (0, 0), flag = wx.EXPAND | wx.TOP, border=0)

		self.SetSizer(sizer)
		self.RegsRefurbish()

	def GetRegsInfo(self):
		print "get a struct that includes reg's name and value."

	def ShowRegsInfo(self):
		print "Show the struct on list."

	def RegsRefurbish(self):
		print "Refurbish when stop skyeye."
		item = 0
		self.RegsList.DeleteAllItems()
		pchar = libcommon.gui_info_register()
		pychar = c_char_p(pchar)
		registers_str = pychar.value
		register_str = registers_str.split(';')
		for i in register_str:
			e = i.split(':')
			if cmp(e[0], '|') != 0 :
				reg_value_10 = "%d" % int(e[1], 16)
				index = self.RegsList.InsertStringItem(item, e[0])
				self.RegsList.SetStringItem(index, 1, e[1].upper())
				self.RegsList.SetStringItem(index, 2, reg_value_10)
				item += 1
		libcommon.info_register_free(pchar)





if __name__ == '__main__':
	app = wx.PySimpleApp()
	Regsdlg = InfoRegsDialog()
	Regsdlg.Show()
	app.MainLoop()
