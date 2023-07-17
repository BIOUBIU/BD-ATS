import numpy as np

dic = {'OSCAR-7':'A','XW-2A':'B','XW-2B':'C','XW-2C':'D','XW-2D':'E','XW-2E':'F','XW-2F':'G','FalconSAT':'H','CAS-4A':'I','CAS-4B':'J','RS-44':'K','OSCAR-50':'L','OSCAR-73':'M','OSCAR-91':'N','OSCAR-92':'O','OSCAR-95':'P','OSCAR-97':'Q','OSCAR-99':'R','NO-104':'S','HO-113':'T','NOAA-18':'U','NOAA-19':'V','HUBBLE':'W','ISS':'X'}

np.save('SatList.npy',dic)