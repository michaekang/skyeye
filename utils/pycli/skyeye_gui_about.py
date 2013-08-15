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


class About(wx.Dialog):
	def __init__(self):
		wx.Dialog.__init__(self, None, -1, C.FontAbout, size=(300, 200), style=wx.DEFAULT_DIALOG_STYLE)
		

		sizer = wx.GridBagSizer(hgap=1, vgap=1)
		font = wx.Font(20, wx.DEFAULT, wx.NORMAL, wx.NORMAL, False, 'Courier New')
		self.Panel = wx.Panel(self)
		str = wx.StaticText(self.Panel, -1, C.FontVersion, (70, 50))
		str.SetFont(font)
		sizer.Add(self.Panel, pos = (0, 0), flag = wx.EXPAND | wx.ALL, border=0)
		self.SetSizer(sizer)
		
