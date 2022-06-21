import json
from ctypes import *
import re
import copy
from struct import unpack
import os
from pluggy._hooks import varnames
import Common.EdkLogger as EdkLogger

class GUID(Structure):
    _fields_ = [
        ('Guid1',            c_uint32),
        ('Guid2',            c_uint16),
        ('Guid3',            c_uint16),
        ('Guid4',            ARRAY(c_uint8, 8)),
    ]

    def from_list(self, listformat):
        self.Guid1 = listformat[0]
        self.Guid2 = listformat[1]
        self.Guid3 = listformat[2]
        for i in range(8):
            self.Guid4[i] = listformat[i+3]

    def __cmp__(self, otherguid):
        if isinstance(otherguid, GUID):
            return 1
        rt = False
        if self.Guid1 == otherguid.Guid1 and self.Guid2 == otherguid.Guid2 and self.Guid3 == otherguid.Guid3:
            rt = True
            for i in range(8):
                rt = rt & (self.Guid4[i] == otherguid.Guid4[i])
        return rt


class TIME(Structure):
    _fields_ = [
        ('Year',             c_uint16),
        ('Month',            c_uint8),
        ('Day',              c_uint8),
        ('Hour',             c_uint8),
        ('Minute',           c_uint8),
        ('Second',           c_uint8),
        ('Pad1',             c_uint8),
        ('Nanosecond',       c_uint32),
        ('TimeZone',         c_uint16),
        ('Daylight',         c_uint8),
        ('Pad2',             c_uint8),
    ]
    def __init__(self):
        self.Year = 0x0
        self.Month = 0x0
        self.Day = 0x0
        self.Hour = 0x0
        self.Minute = 0x0
        self.Second = 0x0
        self.Pad1 = 0x0
        self.Nanosecond = 0x0
        self.TimeZone = 0x0
        self.Daylight = 0x0
        self.Pad2 = 0x0


EFI_VARIABLE_GUID = [0xddcf3616, 0x3275, 0x4164,
                     0x98, 0xb6, 0xfe, 0x85, 0x70, 0x7f, 0xfe, 0x7d]
EFI_AUTHENTICATED_VARIABLE_GUID = [
    0xaaf32c78, 0x947b, 0x439a, 0xa1, 0x80, 0x2e, 0x14, 0x4e, 0xc3, 0x77, 0x92]

AuthVarGuid = GUID()
AuthVarGuid.from_list(EFI_AUTHENTICATED_VARIABLE_GUID)
VarGuid = GUID()
VarGuid.from_list(EFI_VARIABLE_GUID)

# Variable Store Header Format.
VARIABLE_STORE_FORMATTED = 0x5a
# Variable Store Header State.
VARIABLE_STORE_HEALTHY = 0xfe


class VARIABLE_STORE_HEADER(Structure):
    _fields_ = [
        ('Signature',                GUID),
        ('Size',                     c_uint32),
        ('Format',                   c_uint8),
        ('State',                    c_uint8),
        ('Reserved',                 c_uint16),
        ('Reserved1',                c_uint32),
    ]


# Variable data start flag.
VARIABLE_DATA = 0x55AA

# Variable State flags.
VAR_IN_DELETED_TRANSITION = 0xfe
VAR_DELETED = 0xfd
VAR_HEADER_VALID_ONLY = 0x7f
VAR_ADDED = 0x3f


class VARIABLE_HEADER(Structure):
    _fields_ = [
        ('StartId',                  c_uint16),
        ('State',                    c_uint8),
        ('Reserved',                 c_uint8),
        ('Attributes',               c_uint32),
        ('NameSize',                 c_uint32),
        ('DataSize',                 c_uint32),
        ('VendorGuid',               GUID),
    ]


class AUTHENTICATED_VARIABLE_HEADER(Structure):
    _fields_ = [
        ('StartId',                  c_uint16),
        ('State',                    c_uint8),
        ('Reserved',                 c_uint8),
        ('Attributes',               c_uint32),
        ('MonotonicCount',           c_uint64),
        ('TimeStamp',                TIME),
        ('PubKeyIndex',              c_uint32),
        ('NameSize',                 c_uint32),
        ('DataSize',                 c_uint32),
        ('VendorGuid',               GUID),
    ]
    _pack_ = 1


# Alignment of Variable Data Header in Variable Store region.
HEADER_ALIGNMENT = 4


class DEFAULT_INFO(Structure):
    _fields_ = [
        ('DefaultId',                c_uint16),
        ('BoardId',                  c_uint16),
    ]


class DEFAULT_DATA(Structure):
    _fields_ = [
        ('HeaderSize',               c_uint16),
        ('DefaultInfo',              DEFAULT_INFO),
    ]

class DELTA_DATA(Structure):
    _fields_ = [
        ('Offset', c_uint16),
        ('Value', c_uint8),
    ]
    _pack_ = 1

array_re = re.compile(
    "(?P<mType>[a-z_A-Z][a-z_A-Z0-9]*)\[(?P<mSize>[1-9][0-9]*)\]")


class VarField():
    def __init__(self):
        self.Offset = 0
        self.Value = 0
        self.Size = 0

    @property
    def Type(self):
        if self.Size == 1:
            return "UINT8"
        if self.Size == 2:
            return "UINT16"
        if self.Size == 4:
            return "UINT32"
        if self.Size == 8:
            return "UINT64"

        return "UINT8"


BASIC_TYPE = {
    "BOOLEAN": 1,
    "UINT8": 1,
    "UINT16": 2,
    "UINT32": 4,
    "UINT64": 8
}

def GetTypeSize(fieldtype, VarAttributes):
    num = 1
    if "[" in fieldtype:
        num = int(fieldtype.split("[")[1].split("]")[0])
        fieldtype = fieldtype.split("[")[0]
    if fieldtype in VarAttributes:
        return VarAttributes[fieldtype]['TotalSize'] * num
    elif fieldtype in BASIC_TYPE:
        return BASIC_TYPE[fieldtype] * num
    else:
        return None

class CStruct():


    def __init__(self, typedefs):
        self.TypeDefs = typedefs
        self.TypeStack = copy.deepcopy(typedefs)
        self.finalDefs = {}

    def CalStuctSize(self, sType):
        rt = 0
        if sType in BASIC_TYPE:
            return BASIC_TYPE[sType]

        ma = array_re.match(sType)
        if ma:
            mType = ma.group('mType')
            mSize = ma.group('mSize')
            rt += int(mSize) * self.CalStuctSize(mType)
        else:
            for subType in self.TypeDefs[sType]:
                rt += self.CalStuctSize(subType['Type'])

        return rt

    def expend(self, fielditem):
        fieldname = fielditem['Name']
        fieldType = fielditem['Type']
        fieldOffset = fielditem['Offset']

        ma = array_re.match(fieldType)
        if ma:
            mType = ma.group('mType')
            mSize = ma.group('mSize')
            return [{"Name": "%s[%d]" % (fieldname, i), "Type": mType, "Offset": (fieldOffset + i*self.CalStuctSize(mType))} for i in range(int(mSize))]
        else:
            return [{"Name": "%s.%s" % (fieldname, item['Name']), "Type":item['Type'], "Offset": (fieldOffset + item['Offset'])} for item in self.TypeDefs[fielditem['Type']]]

    def ExpandTypes(self):
        if not self.finalDefs:
            for datatype in self.TypeStack:
                result = []
                mTypeStack = self.TypeStack[datatype]
                while len(mTypeStack) > 0:
                    item = mTypeStack.pop()
                    if item['Type'] in self.BASIC_TYPE:
                        result.append(item)
                    elif item['Type'] == '(null)':
                        continue
                    else:
                        for expand_item in self.expend(item):
                            mTypeStack.append(expand_item)
                self.finalDefs[datatype] = result
            self.finalDefs
        return self.finalDefs

def Get_Occupied_Size(FileLength, alignment):
    if FileLength % alignment == 0:
        return FileLength
    return FileLength + (alignment-(FileLength % alignment))

def Occupied_Size(buffer, alignment):
    FileLength = len(buffer)
    if FileLength % alignment != 0:
        buffer += b'\0' * (alignment-(FileLength % alignment))
    return buffer

def PackStruct(cStruct):
    length = sizeof(cStruct)
    p = cast(pointer(cStruct), POINTER(c_char * length))
    return p.contents.raw

def calculate_delta(default, theother):

    if len(default) - len(theother) != 0:
        return []

    data_delta = []
    for i in range(len(default)):
        if default[i] != theother[i]:
            data_delta.append([i, theother[i]])
    return data_delta

class Variable():
    def __init__(self):
        self.mAlign = 1
        self.mTotalSize = 1
        self.mValue = {}  # {defaultstore: value}
        self.mBin = {}
        self.fields = {}  # {defaultstore: fileds}
        self.delta = {}
        self.attributes = 0
        self.mType = ''
        self.guid = ''
        self.mName = ''
        self.cDefs = None
        self.Struct = None
        self.TypeList = {}

    @property
    def GuidArray(self):

        guid_array = []
        guid = self.guid.strip().strip("{").strip("}")
        for item in guid.split(","):
            field = item.strip().strip("{").strip("}")
            guid_array.append(int(field,16))
        return guid_array

    def update_delta_offset(self,base):
        for default_id in self.delta:
            for delta_list in self.delta[default_id]:
                delta_list[0] += base

    def pack(self):

        for defaultid in self.mValue:
            var_value = self.mValue[defaultid]
            auth_var = AUTHENTICATED_VARIABLE_HEADER()
            auth_var.StartId = VARIABLE_DATA
            auth_var.State = VAR_ADDED
            auth_var.Reserved = 0x00
            auth_var.Attributes = 0x00000007
            auth_var.MonotonicCount = 0x0
            auth_var.TimeStamp = TIME()
            auth_var.PubKeyIndex = 0x0
            var_name_buffer = self.mName.encode('utf-16le') + b'\0\0'
            auth_var.NameSize = len(var_name_buffer)
            auth_var.DataSize = len(var_value)
            vendor_guid = GUID()
            vendor_guid.from_list(self.GuidArray)
            auth_var.VendorGuid = vendor_guid

            self.mBin[defaultid] = PackStruct(auth_var) + Occupied_Size(var_name_buffer + var_value, 4)

    def TypeCheck(self,data_type, data_size):
        if BASIC_TYPE[data_type] == data_size:
            return True
        return False

    def ValueToBytes(self,data_type,data_value,data_size):

        rt = b''
        if not self.TypeCheck(data_type, data_size):
            print(data_type,data_value,data_size)

        if data_type == "BOOLEAN" or data_type == 'UINT8':
            p = cast(pointer(c_uint8(int(data_value,16))), POINTER(c_char * 1))
            rt = p.contents.raw
        elif data_type == 'UINT16':
            p = cast(pointer(c_uint16(int(data_value,16))), POINTER(c_char * 2))
            rt = p.contents.raw
        elif data_type == 'UINT32':
            p = cast(pointer(c_uint32(int(data_value,16))), POINTER(c_char * 4))
            rt = p.contents.raw
        elif data_type == 'UINT64':
            p = cast(pointer(c_uint64(int(data_value,16))), POINTER(c_char * 8))
            rt = p.contents.raw

        return rt

    def serial(self):
        for defaultstore in self.fields:
            vValue = b''
            vfields = {vf.Offset: vf for vf in self.fields[defaultstore]}
            i = 0
            while i < self.mTotalSize:
                if i in vfields:
                    vfield = vfields[i]
                    if vfield.Size != 0:
                        vValue += self.ValueToBytes(vfield.Type, vfield.Value,vfield.Size)
                        i += vfield.Size
                else:
                    vValue += self.ValueToBytes('UINT8','0x00',1)
                    i += 1

            self.mValue[defaultstore] = vValue
        standard_default = self.mValue[0]

        for defaultid in self.mValue:
            if defaultid == 0:
                continue
            others_default = self.mValue[defaultid]

            self.delta.setdefault(defaultid, []).extend(calculate_delta(
                standard_default, others_default))

class DefaultVariableGenerator():
    def __init__(self):
        self.NvVarInfo = []

    def LoadNvVariableInfo(self, VarInfoFilelist):

        VarDataDict = {}
        DataStruct = {}
        VarDefine = {}
        VarAttributes = {}
        for VarInfoFile in VarInfoFilelist:
            with open(VarInfoFile.strip(), "r") as fd:
                data = json.load(fd)

            DataStruct.update(data.get("DataStruct", {}))
            Data = data.get("Data")
            VarDefine.update(data.get("VarDefine"))
            VarAttributes.update(data.get("DataStructAttribute"))

            for vardata in Data:
                if vardata['VendorGuid'] == 'NA':
                    continue
                VarDataDict.setdefault(
                    (vardata['VendorGuid'], vardata["VarName"]), []).append(vardata)

        cStructDefs = CStruct(DataStruct)
        for guid, varname in VarDataDict:
            v = Variable()
            v.guid = guid
            vardef = VarDefine.get(varname)
            if vardef is None:
                for var in VarDefine:
                    if VarDefine[var]['Type'] == varname:
                        vardef = VarDefine[var]
                        break
                else:
                    continue
            v.attributes = vardef['Attributes']
            v.mType = vardef['Type']
            v.mAlign = VarAttributes[v.mType]['Alignment']
            v.mTotalSize = VarAttributes[v.mType]['TotalSize']
            v.Struct = DataStruct[v.mType]
            v.mName = varname
            v.cDefs = cStructDefs
            v.TypeList = VarAttributes
            for fieldinfo in VarDataDict.get((guid, varname), []):
                vf = VarField()
                vf.Offset = fieldinfo['Offset']
                vf.Value = fieldinfo['Value']
                vf.Size = fieldinfo['Size']
                v.fields.setdefault(
                    int(fieldinfo['DefaultStore'], 10), []).append(vf)
            v.serial()
            v.pack()
            self.NvVarInfo.append(v)

    def PackDeltaData(self):

        default_id_set = set()
        for v in self.NvVarInfo:
            default_id_set |= set(v.mBin.keys())

        if default_id_set:
            default_id_set.remove(0)
        delta_buff_set = {}
        for defaultid in default_id_set:
            delta_buff = b''
            for v in self.NvVarInfo:
                delta_list = v.delta.get(defaultid,[])
                for delta in delta_list:
                    delta_data = DELTA_DATA()
                    delta_data.Offset, delta_data.Value = delta
                    delta_buff += PackStruct(delta_data)
            delta_buff_set[defaultid] = delta_buff

        return delta_buff_set

    def PackDefaultData(self):

        default_data_header = DEFAULT_DATA()
        default_data_header.HeaderSize = sizeof(DEFAULT_DATA)
        default_data_header.DefaultInfo.DefaultId = 0x0
        default_data_header.DefaultInfo.BoardId = 0x0
        default_data_header_buffer = PackStruct(default_data_header)


        variable_store = VARIABLE_STORE_HEADER()
        variable_store.Signature = AuthVarGuid

        variable_store_size = Get_Occupied_Size(sizeof(DEFAULT_DATA) + sizeof(VARIABLE_STORE_HEADER), 4)
        for v in self.NvVarInfo:
            variable_store_size += Get_Occupied_Size(len(v.mBin[0]), 4)

        variable_store.Size = variable_store_size
        variable_store.Format = VARIABLE_STORE_FORMATTED
        variable_store.State = VARIABLE_STORE_HEALTHY
        variable_store.Reserved = 0x0
        variable_store.Reserved2 = 0x0

        variable_storage_header_buffer = PackStruct(variable_store)

        variable_data = b''
        v_offset = 0
        for v in self.NvVarInfo:
            v.update_delta_offset(v_offset)
            variable_data += Occupied_Size(v.mBin[0],4)
            v_offset += Get_Occupied_Size(len(v.mBin[0]),4)


        final_buff = Occupied_Size(default_data_header_buffer + variable_storage_header_buffer,4) + variable_data

        return final_buff

    def GenVariableInfo(self, build_macro):
        VariableInfo = []
        VariableInfo.append('Variable Information Report\n')
        if build_macro:
            VariableInfo.append('[Platform Definitions]')
            for define_item in build_macro:
                if '=' in define_item:
                    VariableInfo.append('* %-20s= %s'%(define_item.split('=')[0], define_item.split('=')[1]))
                else:
                    VariableInfo.append('* ' + define_item)
        VariableInfo.append('\n[Variable List]')
        VariableInfo.append('# {} variables used in current setting:'.format(len(self.NvVarInfo)))
        for item in self.NvVarInfo:
            VariableInfo.append('* ' + item.mName)
        VariableInfo.append('\n[Variable Details]')
        for item in self.NvVarInfo:
            VariableInfo.append('####################')
            VariableInfo.append('* Variable Name: ' + item.mName)
            VariableInfo.append('* Variable Type: ' + item.mType)
            VariableInfo.append('* Variable Guid: ' + item.guid)
            VariableInfo.append('* Variable Size: ' + hex(item.mTotalSize))
            
            ## Field structure Info
            VariableInfo.append('* Variable Fields: {} fields'.format(len(item.Struct)))
            VariableInfo.append('{')
            VariableInfo.append('# %-5s | %-30s | %-15s | %-5s | %s'%('Index', 'Name', 'Type', 'TotalSize', 'Offset'))
            FieldsNum = len(item.Struct)
            Name_Offset = {}
            if FieldsNum == 0:
                Name_Offset[0] = field['Name']
            for i in range(FieldsNum):
                field = item.Struct[i]
                Name_Offset[field['Offset']] = field['Name']
                if i != FieldsNum-1:
                    nextfield = item.Struct[i+1]
                    for cur_offset in range(field['Offset']+1, nextfield['Offset']):
                        Name_Offset[cur_offset] = field['Name']
                VariableInfo.append('  %-5s | %-30s | %-15s | %-5s | %s'%(i, field['Name'], field['Type'], GetTypeSize(field['Type'], item.TypeList), field['Offset']))
            VariableInfo.append('}')
            
            ## Field value Info
            VariableInfo.append('* Field value: ')
            VariableInfo.append('{')
            VariableInfo.append('# defaultstore %-5s | %-30s | %-15s | %-15s | %s'%('xxxx', 'FieldName', 'FieldType', 'FieldOffset', 'FieldValue'))
            storenum = len(item.fields)
            for storeid in range(storenum):
                for Var in item.fields[storeid]:
                    print('If Var.Offset in Name_Offset: ', Var.Offset in Name_Offset)
                    if Var.Offset not in Name_Offset:
                        print('Offset:', Var.Offset)
                    VariableInfo.append('  defaultstore %-5s | %-30s | %-15s | %-15s | %s'%(storeid, Name_Offset[Var.Offset], Var.Type, Var.Offset, Var.Value))
            VariableInfo.append('}\n')
        return VariableInfo

    def generate(self, jsonlistfile, output_folder, build_macro=None):
        if not os.path.exists(jsonlistfile):
            return
        if not os.path.exists(output_folder):
            os.makedirs(output_folder)
        try:
            with open(jsonlistfile,"r") as fd:
                filelist = fd.readlines()
            genVar = DefaultVariableGenerator()
            genVar.LoadNvVariableInfo(filelist)
            with open(os.path.join(output_folder, "default.bin"), "wb") as fd:
                fd.write(genVar.PackDefaultData())

            delta_set = genVar.PackDeltaData()
            for default_id in delta_set:
                with open(os.path.join(output_folder, "defaultdelta_%s.bin" % default_id), "wb") as fd:
                    fd.write(delta_set[default_id])

            VariableInfo = genVar.GenVariableInfo(build_macro)
            if VariableInfo:
                with open(os.path.join(output_folder, "VariableInfo.txt"), "w") as reportfile:
                    for item in VariableInfo:
                        reportfile.write(item + '\n')
        except:
            EdkLogger.info("generate varbin file failed")
