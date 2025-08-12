import struct

# Ethernet header (14 bytes)
class ethheader:
    @staticmethod
    def read(buf, offset):
        eth_header = struct.unpack('!6s6sH', buf[offset:offset + 14])  # 6 bytes for source and destination MAC, 2 bytes for EtherType
        ethh = ethheader()
        ethh.dst_mac = eth_header[0]
        ethh.src_mac = eth_header[1]
        ethh.ethtype = eth_header[2]
        return ethh

# VLAN header (4 bytes)
class vlanheader:
    @staticmethod
    def read(buf, offset):
        vlan_header = struct.unpack('!HH', buf[offset:offset + 4])  # 2 bytes for TPID, 2 bytes for other fields
        vlanh = vlanheader()
        vlanh.extrafields = vlan_header[0]  # TPID
        vlanh.tpid = vlan_header[1]  # 2B extra field
        return vlanh

# eCPRI header (example, 18 bytes)
class ecpriheader:
    @staticmethod
    def read(buf, offset):
        # Assuming the eCPRI header structure is at offset 18
        ecpri_header = struct.unpack('!BBH ', buf[offset:offset + 4])  # example structure: payload_type, etc.
        ecpri = ecpriheader()
        ecpri.payload_type = ecpri_header[1]  # For example, this can represent the message type
        return ecpri

# OFH header (example, 22 bytes)
class ofhheader:
    @staticmethod
    def read(buf, offset):
        # Assuming the OFH header structure starts at byte 22
        ofh_header = struct.unpack('!H', buf[offset:offset + 2])  # placeholder
        ofh = ofhheader()
        ofh.some_field = ofh_header[0]  # example placeholder field
        return ofh
