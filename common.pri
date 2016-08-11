DEFINES += QT_MESSAGELOGCONTEXT

isEmpty(PREFIX){
    PREFIX = /usr
}

ARCH = $$QMAKE_HOST.arch
isEqual(ARCH, mips64) | isEqual(ARCH, mips32) | isEqual(ARCH, sw_64) {
    DEFINES += ARCH_MIPSEL  #TODO: remove asap.
    DEFINES += DISABLE_DEVICE_MONITER
}

include(interfaces/interfaces.pri)
