# -*- coding: utf-8 -*-
import wx, platform, sys, os, string
from wx.lib import masked
import  wx.lib.mixins.listctrl  as  listmix
from ctypes import *

os_info = platform.system()

if cmp(os_info, "Linux"):
	from skyeye_common_windows_module import *
	import windows_font as C
else:
	from skyeye_common_module import *
	import linux_font as C


class TestListCtrl(wx.ListCtrl, 
                   listmix.TextEditMixin):

    def __init__(self, parent, ID, pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=0):
        wx.ListCtrl.__init__(self, parent, ID, pos, size, style)
        
        listmix.TextEditMixin.__init__(self)
#        font = wx.SystemSettings_GetFont(wx.SYS_SYSTEM_FONT)
#        font.SetPointSize(18)
#        self.SetFont(font)
    
    def OpenEditor(self, col, row):
        if col>1:
            listmix.TextEditMixin.OpenEditor(self,col, row)
            
    def CloseEditor(self, evt=None):
        listmix.TextEditMixin.CloseEditor(self, evt)
        item = self.GetItem(self.curRow,col=1)
        oldname = item.GetText()
        item = self.GetItem(self.curRow,col=2)
        newname = item.GetText()
        if oldname != newname:
            self.SetItemBackgroundColour(self.curRow,"pink")
        else:
            self.SetItemBackgroundColour(self.curRow,"white")
 




class Memory(wx.Frame):
		
	def __init__(self):
		wx.Frame.__init__(self, None, -1, C.FontMems, size=(580, 410), style=wx.DEFAULT_DIALOG_STYLE|wx.MAXIMIZE_BOX|wx.MINIMIZE_BOX)
        	sizer = wx.GridBagSizer(hgap=5, vgap=5)
		basicLabel = wx.StaticText(self, -1, C.FontAddr)
		sizer.Add(basicLabel, pos = (0, 0), flag = wx.LEFT|wx.TOP, border=10)

		self.pwdText = wx.TextCtrl(self, -1, "", style=wx.TE_PROCESS_ENTER)
		sizer.Add(self.pwdText, pos = (0, 1), flag = wx.EXPAND|wx.TOP, border=10)

		self.Bind(wx.EVT_TEXT_ENTER, self.InputAddr, self.pwdText)

		groupbox = wx.StaticBox(self, -1, C.FontMems, name = "groupbox2_sbox")
        	groupsizer = wx.StaticBoxSizer(groupbox, wx.VERTICAL)



		self.list = TestListCtrl(self, -1,  style=wx.LC_REPORT|wx.LC_HRULES)
        	self.list.InsertColumn(0, C.FontAddr, format=wx.LIST_FORMAT_LEFT, width=100)
        	self.list.InsertColumn(1, " 0   1   2   3    4   5   6   7    8   9   A   B    C   D   E   F",format=wx.LIST_FORMAT_LEFT, width = 375)
        	groupsizer.Add(self.list, 1, wx.EXPAND|wx.TOP, 10)

        	sizer.Add(groupsizer, pos=(1,0),span=(6,2), flag=wx.EXPAND|wx.LEFT|wx.TOP|wx.BOTTOM,border=10)


		ok_button = wx.Button(self, -1, C.FontOK, size = (50, 30), name = "ok_button")
        	sizer.Add(ok_button, pos=(0,2),flag=wx.ALIGN_BOTTOM|wx.TOP|wx.RIGHT,border=10)
		self.Bind(wx.EVT_BUTTON, self.InputAddr, ok_button)

		up_button = wx.Button(self,-1, C.FontUp, size = (50, 30), name= "up_button")
        	sizer.Add(up_button, pos=(5,2),flag=wx.ALIGN_BOTTOM|wx.TOP|wx.RIGHT,border=10)
		self.Bind(wx.EVT_BUTTON, self.Up, up_button)
		self.Bind(wx.EVT_LEFT_DOWN, self.Up, up_button)
        
        	down_button = wx.Button(self,-1, C.FontDown, size = (50, 30), name= "down_button")
        	sizer.Add(down_button, pos=(6,2),flag=wx.ALIGN_CENTER|wx.TOP|wx.RIGHT|wx.BOTTOM,border=10)
		self.Bind(wx.EVT_BUTTON, self.Down, down_button)
		


		sizer.AddGrowableCol(1)
		sizer.AddGrowableRow(2)
		self.SetSizer(sizer)
		self.Center()
		self.statusbar = self.CreateStatusBar()
		self.statusbar.SetFieldsCount(2)
		self.statusbar.SetStatusWidths([-1, -2])
		self.statusbar.SetStatusText("PC:", 0)
		self.statusbar.SetStatusText("Value:", 1)

        

	def Init(self):
		self.list.DeleteAllItems()
		self.maxitem = 0    #最大item
		self.maxaddrvalue  = 0  #最大地址值
		self.minaddrvalue  = 0 #最小地址值
		self.colitem = 6 #当前输入的地址


	def HandleLineStr(self, inputstr):
		show_value = ""
		bytestr = ""
		str = inputstr.upper()
		num = len(str)
		for i in range(0, (8 - num)):
			str = "0" + str

		for i in range(8, 0, -2):
			bytestr = str[i-2: i]
			bytestr = "%03s" % bytestr 
			show_value += bytestr
		show_value = "%13s" % show_value
			
		return show_value




	def HandleLineValue(self, startvalue):
		value_str_all = ""
		for i in range(4):
			arg = "0x%x" % (startvalue + i*4)
			arg_item = " %x" % (startvalue + i * 4)
			arg_str = c_char_p(arg)
			libcommon.gui_x.restpye = c_uint
			value = libcommon.gui_x(arg_str) & 0xffffffff
			value_str = "%x" % value
			value_str_all += self.HandleLineStr(value_str)
		return value_str_all
	


		
		
	def Refreshput(self):
		self.Init()
		print self.global_input 
		start_value = self.global_input - 16* 6
		self.minaddrvalue = start_value
		self.maxaddrvalue = self.global_input + 16 * 6
		for i in range(13):
			arg_item = " %x" % (start_value + i * 16)
			value_str = self.HandleLineValue(start_value + i * 16)
			index = self.list.InsertStringItem(self.maxitem, arg_item.upper())
			self.list.SetStringItem(index, 1, value_str)
			self.list.SetItemTextColour(self.colitem, wx.RED)
			self.maxitem += 1



	def InputAddr(self, event):
		self.Init()
		self.global_input = 0
		input_str = self.pwdText.GetLineText(0)
		self.pwdText.Clear()
		input_value = int(input_str, 16)
		self.global_input = input_value
		start_value = input_value - 16* 6
		self.minaddrvalue = start_value
		self.maxaddrvalue = input_value + 16 * 6
		for i in range(13):
			arg_item = " %x" % (start_value + i * 16)
			value_str = self.HandleLineValue(start_value + i * 16)
			index = self.list.InsertStringItem(self.maxitem, arg_item.upper())
			self.list.SetStringItem(index, 1, value_str)
			self.list.SetItemTextColour(self.colitem, wx.RED)
			self.maxitem += 1

			
	def Up(self, event):
		self.minaddrvalue -= 16
		arg_item = " %x" % self.minaddrvalue
		value_str = self.HandleLineValue(self.minaddrvalue)
		index = self.list.InsertStringItem(0, arg_item.upper())
		self.list.SetStringItem(index, 1, value_str)
		colour = self.list.GetItemTextColour(0)
		self.list.SetItemTextColour(self.colitem, colour)
		self.colitem += 1
		self.list.SetItemTextColour(self.colitem, wx.RED)
		self.maxitem += 1

	


	def Down(self, event): 
		self.maxaddrvalue += 16
		arg_item = " %x" % self.maxaddrvalue
		value_str = self.HandleLineValue(self.maxaddrvalue)
		index = self.list.InsertStringItem(self.maxitem, arg_item.upper())
		self.list.SetStringItem(index, 1, value_str)
		self.maxitem += 1
		
	






