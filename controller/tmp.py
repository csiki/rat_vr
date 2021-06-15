import serial as ser
import serial.tools.list_ports as prtlst
import usb

# TODO install libusb: https://github.com/pyusb/pyusb
#   do these in linux, not windows pls

# TODO then get the usb serial numbers for both arduinos, so you can send msgs to them selectively
#   regardless of their com port

# com stuff
def getCOMs():
    pts = prtlst.comports()

    coms = []
    for pt in pts:
        print(pt)
        if 'Arduino' in pt[1]:  # check 'USB' string in device description
            coms.append(pt[0])

    return coms

print(getCOMs())

# usb stuff
dev = usb.core.find(find_all=True)
for d in dev:
    print(usb.util.get_string(d,128,d.iManufacturer))
    print(usb.util.get_string(d,128,d.iProduct))
    print((d.idProduct,d.idVendor))

