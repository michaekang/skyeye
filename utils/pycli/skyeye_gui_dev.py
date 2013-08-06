# -*- coding: UTF-8 -*-

import wx
import os
import sys, platform

os_info = platform.system()
if cmp(os_info, "Linux"):
	import windows_font as C
else:
	import linux_font as C
regs1_value = {"reg1":0x1, "reg2":2, "reg3":3, "reg4":4, "reg5":5, "reg6":6, "reg7":7, "reg8":8, "reg9":9, "reg0":0xf}
regs2_value = {"reg1":10, "reg2":12, "reg3":13, "reg4":14, "reg5":15, "reg6":16, "reg7":17, "reg8":18, "reg9":19, "reg0":20}
regs3_value = {"reg1":10, "reg2":12, "reg3":13, "reg4":14, "reg5":15, "reg6":16, "reg7":17, "reg8":18, "reg9":19, "reg0":20}
dict = {"dev1":regs1_value, "dev2":regs2_value, "dev3":regs3_value}
devlist = ["dev1", "dev2", "dev3", "dev1", "dev2", "dev3", "dev1", "dev2", "dev3", "dev1", "dev2", "dev3", "dev1", "dev2", "dev3", "dev1", "dev2", "dev3", "dev1", "dev2", "dev3","dev1", "dev2", "dev3", "dev1", "dev2", "dev3", "dev1", "dev2", "dev3", "dev1", "dev2", "dev3", "dev1", "dev2", "dev3", "dev1", "dev2", "dev3", "dev1", "dev2", "dev3"]
class DevRegsFrame(wx.Frame):
	ShowDevRegs = None
	def __init__(self):
		wx.Frame.__init__(self, None, -1, C.FontDeviceList, size=(500, 500))
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
		self.ShowDevRegs = dict[DevName]
		self.DevRegsList.DeleteAllItems()
		index = 2
		for key in self.ShowDevRegs:
			index = self.DevRegsList.InsertStringItem(sys.maxint, key)
			self.DevRegsList.SetStringItem(index, 0, key)
			self.DevRegsList.SetStringItem(index, 1, str(hex(self.ShowDevRegs[key])))
			index = index + 1

if __name__ == '__main__':
	app = wx.PySimpleApp()
	Regsdlg = DevRegsFrame()
	Regsdlg.Show()
	app.MainLoop()
