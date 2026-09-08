uint32_t sceGsSetDefLoadImage(
    Ps2GsLoadImagePacket* packet,
    int dbp,
    int dbw,
    int dpsm,
    int dsax,
    int dsay,
    int rrw,
    int rrh
)
{
    uint64_t transfer_size = 0;

    switch (dpsm) {
    case 0:
    case 0x30:
        transfer_size = (uint64_t)(rrw * rrh >> 2);
        break;

    case 1:
    case 0x31:
        transfer_size = (uint64_t)(rrw * rrh * 3 >> 4);
        break;

    case 2:
    case 10:
    case 0x32:
    case 0x3a:
        transfer_size = (uint64_t)(rrw * rrh >> 3);
        break;

    case 0x13:
    case 0x1b:
        transfer_size = (uint64_t)(rrw * rrh >> 4);
        break;

    case 0x14:
    case 0x24:
    case 0x2c:
        transfer_size = (uint64_t)(rrw * rrh >> 5);
        break;
    }

    if (transfer_size >= 0x8000) {
        boot_txt_render_extended_string(
            (const uint8_t*)
            "sceGsSetDefLoadImage: too big size\r\n",
            (int64_t)(dbp << 16),
            transfer_size,
            dsax,
            rrh,
            (int64_t)(dsay << 16),
            rrw,
            (int64_t)(rrh << 16)
        );

        return 0;
    }

    /*
     * Inicialización del paquete.
     */
    for (int i = 0; i < 12; ++i) {
        packet->qword[i] = 0;
    }

    /*
     * BITBLTBUF
     */
    packet->qword[2] =
        ((uint64_t)(dbp & 0xFFFF) << 32) |
        ((uint64_t)(dbw & 0xFFFF) << 48) |
        ((uint64_t)(dpsm & 0xFF) << 56);

    packet->qword[3] = 0x50;

    /*
     * TRXPOS
     */
    packet->qword[4] =
        ((uint64_t)(dsax & 0xFFFF) << 32) |
        ((uint64_t)(dsay & 0xFFFF) << 48);

    packet->qword[5] = 0x51;

    /*
     * TRXREG
     */
    packet->qword[6] =
        ((uint64_t)(rrw & 0xFFFF)) |
        ((uint64_t)(rrh & 0xFFFF) << 32);

    packet->qword[7] = 0x52;

    /*
     * TRXDIR
     *
     * 0 = host -> local
     */
    packet->qword[8] = 0;
    packet->qword[9] = 0x53;

    ps2_sync(0);

    return 6;
}