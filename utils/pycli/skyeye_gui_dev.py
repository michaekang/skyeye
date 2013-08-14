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

devlist = ["uart_0", "am35x_hecc_0"]
class DevRegsFrame(wx.Frame):
	ShowDevRegs = None
	def __init__(self):
		wx.Frame.__init__(self, None, -1, C.FontDeviceList, size=(500, 532))
		self.Panel = wx.Panel(self)
		MainSizer = wx.BoxSizer(wx.VERTICAL)
		DevSizer = wx.BoxSizer(wx.HORIZONTAL)
		#Add the TIP to choice device
		self.ChoiceTip = wx.StaticText(self.Panel, -1, C.FontChoiceDev)
		font = wx.Font(11, wx.DECORATIVE, wx.NORMAL, wx.LIGHT)
		self.ChoiceTip.SetFont(font)
		DevSizer.Add(self.ChoiceTip, 0, wx.EXPAND)
		self.DevList = wx.ComboBox(self.Panel, -1, value = "Choice Device", choices = devlist, style = wx.CB_READONLY, size = (200, 30))
		self.Bind(wx.EVT_TEXT, self.RegsRefurbish, self.DevList)
		DevSizer.Add(self.DevList, 0, wx.EXPAND)
		MainSizer.Add(DevSizer, 0, wx.EXPAND)

		#Add the List to show device's regiesters
		self.DevRegsList = wx.ListCtrl(self.Panel, -1, style = wx.LC_REPORT | wx.LC_HRULES, size = (400, 500))
		self.DevRegsList.InsertColumn(0, C.FontReg, format = wx.LIST_FORMAT_LEFT, width = 200)
		self.DevRegsList.InsertColumn(1, C.FontValue, format = wx.LIST_FORMAT_LEFT, width = 300)
		MainSizer.Add(self.DevRegsList, 0, wx.EXPAND)
		self.Panel.SetSizer(MainSizer)


	def GetRegsInfo(self):
		print "get a struct that includes reg's name and value."

	def ShowRegsName(self):
		print "Show the struct on list."

	def ShowRegsValue(self):
		print "Show the struct on list."

	def RegsRefurbish(self, event):
		DevName = self.DevList.GetValue()
		self.DevRegsList.DeleteAllItems()
		reg_id = 0;
		libcommon.dev_get_regname_by_id.restype = c_char_p
		libcommon.dev_get_regvalue_by_id.restype = c_uint
		while(True):
			RegName = libcommon.dev_get_regname_by_id(c_char_p(DevName), c_int(reg_id))
			if(RegName == None):
				break;
			RegValue = libcommon.dev_get_regvalue_by_id(c_char_p(DevName), c_int(reg_id))
			print RegName, RegValue
			index = self.DevRegsList.InsertStringItem(sys.maxint, DevName)
			self.DevRegsList.SetStringItem(index, 0, RegName)
			self.DevRegsList.SetStringItem(index, 1, str(hex(RegValue)))
			reg_id = reg_id + 1

if __name__ == '__main__':
	app = wx.PySimpleApp()
	Regsdlg = DevRegsFrame()
	Regsdlg.Show()
	app.MainLoop()
