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
	import linux_font as C
	from skyeye_common_module import *


class TestListCtrl(wx.ListCtrl, 
                   listmix.TextEditMixin):

    def __init__(self, parent, ID, pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=0):
        wx.ListCtrl.__init__(self, parent, ID, pos, size, style)
        
        listmix.TextEditMixin.__init__(self)
    
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
 




class Disassembler(wx.Dialog):
	global_input = 0
	MaxLine = 24
		
	def __init__(self):
		wx.Dialog.__init__(self, None, -1, C.FontDisassembler, size=(570, 600), style=wx.DEFAULT_DIALOG_STYLE|wx.MAXIMIZE_BOX|wx.MINIMIZE_BOX)
        	sizer = wx.GridBagSizer(hgap=5, vgap=5)
		basicLabel = wx.StaticText(self, -1, C.FontAddr)
		sizer.Add(basicLabel, pos = (0, 0), flag = wx.LEFT|wx.TOP, border=10)

		self.pwdText = wx.TextCtrl(self, -1, "", style=wx.TE_PROCESS_ENTER)
		sizer.Add(self.pwdText, pos = (0, 1), flag = wx.EXPAND|wx.TOP, border=10)

		self.Bind(wx.EVT_TEXT_ENTER, self.InputAddr, self.pwdText)

		groupbox = wx.StaticBox(self, -1, C.FontDisassembler, name = "groupbox2_sbox")
        	groupsizer = wx.StaticBoxSizer(groupbox, wx.VERTICAL)



		self.list = TestListCtrl(self, -1,  style=wx.LC_REPORT|wx.LC_HRULES)
        	self.list.InsertColumn(0, C.FontAddr, format=wx.LIST_FORMAT_LEFT, width=150)
        	self.list.InsertColumn(1, C.FontDisassembler,format=wx.LIST_FORMAT_LEFT, width = 345)
        	groupsizer.Add(self.list, 1, wx.EXPAND|wx.TOP, 10)

        	sizer.Add(groupsizer, pos=(1,0),span=(6,2), flag=wx.EXPAND|wx.LEFT|wx.TOP|wx.BOTTOM,border=10)

		
		ok_button = wx.Button(self, -1, C.FontOK, size = (60, 30), name = "ok_button")
        	sizer.Add(ok_button, pos=(0,2),flag=wx.ALIGN_BOTTOM|wx.TOP|wx.RIGHT,border=10)
		self.Bind(wx.EVT_BUTTON, self.InputAddr, ok_button)

        	StepiBtn = wx.Button(self,-1, C.FontNextStep, size = (60, 30), name= "stepi_button")
        	sizer.Add(StepiBtn, pos=(1,2),flag=wx.ALIGN_CENTER|wx.TOP|wx.RIGHT|wx.BOTTOM,border=10)
		self.Bind(wx.EVT_BUTTON, self.Stepi, StepiBtn)
		
		sizer.AddGrowableCol(1)
		sizer.AddGrowableRow(2)
		self.SetSizer(sizer)
		self.Center()
		# set the C function's return value
		libdisasm.read_web_disassemble_buf.restype = c_char_p
		libcommon.com_get_pc.restype = c_uint

		self.Refurbish()

	def Init(self):
		self.pwdText.Clear()
		self.list.DeleteAllItems()
		self.Cur_PC_off = 0
		self.Pre_PC_off = 0 #Pre PC value for restoring the colour

	def Refurbish(self):
		self.Init()
		pc_value = libcommon.com_get_pc()
                for i in range(self.MaxLine):
                        libdisasm.clear_web_disassemble_buf()
                        libdisasm.web_disassemble(c_uint(pc_value))
                        disas_str = libdisasm.read_web_disassemble_buf()
                        # Show the addr and disasemble
                        index = self.list.InsertStringItem(sys.maxint, "NULL")
                        self.list.SetStringItem(index, 0, str(hex(pc_value)))
                        self.list.SetStringItem(index, 1, disas_str)
                        pc_value = pc_value + 4
		pc_value = libcommon.com_get_pc()
		self.Cur_PC_off = self.list.FindItem(-1, str(hex(pc_value)))
		self.list.SetItemTextColour(self.Cur_PC_off, wx.RED)
		self.Pre_PC_off = self.Cur_PC_off

	def InputAddr(self, event):
		addr_str = self.pwdText.GetLineText(0)
		self.Init()
		addr_value = int(addr_str, 16)
		for i in range(self.MaxLine):
			libdisasm.web_disassemble(c_uint(addr_value))
			disas_str = libdisasm.read_web_disassemble_buf()
			# Show the addr and disasemble
			index = self.list.InsertStringItem(sys.maxint, "NULL")
			self.list.SetStringItem(index, 0, str(hex(addr_value)))
			self.list.SetStringItem(index, 1, disas_str)
			libdisasm.clear_web_disassemble_buf()
			addr_value = addr_value + 4

	def Stepi(self, event): 
		libcommon.com_si(c_char_p(None))
		pc_value = libcommon.com_get_pc()
		self.Cur_PC_off = self.list.FindItem(-1, str(hex(pc_value)))
		if self.Cur_PC_off != -1:
			self.list.SetItemTextColour(self.Pre_PC_off, wx.BLACK)
			self.list.SetItemTextColour(self.Cur_PC_off, wx.RED)
			self.Pre_PC_off = self.Cur_PC_off
			return;
		self.Refurbish()
