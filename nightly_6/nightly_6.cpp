/*
解析一个二进制帧（含大小端字段）

帧格式（固定 22 字节）：
[0..1]  magic        2B   固定 0xAA 0x55
[2]     type         1B
[3..4]  len          2B   小端 LE
[5..8]  seq          4B   小端 LE
[9..12] pressure     4B   大端 BE
[13..14] temp_x100   2B   大端 BE（有符号，温度=值/100）
[15..16] voltage_mV  2B   小端 LE
[17..20] yaw_deg_x10 4B   大端 BE（有符号，偏航角=值/10）
[21]    tail         1B   固定 0x0D
*/

#include <cstdint>
#include <iostream>

using namespace std;

//le:小端序
//be：大端序
static uint16_t le16(const uint8_t* p){
    return uint16_t(p[0]) | (uint16_t(p[1]) << 8);
}

static uint32_t le32(const uint8_t* p){
    return uint32_t(p[0]) | 
    (uint32_t(p[1]) << 8) | 
    (uint32_t(p[2]) << 16) | 
    (uint32_t(p[3]) << 24);
}

static uint16_t be16(const uint8_t* p){
    return uint16_t(p[0] << 8) | (uint16_t(p[1]));
}

static uint32_t be32(const uint8_t* p){
    return uint32_t(p[0] << 24) | 
    (uint32_t(p[1]) << 16) | 
    (uint32_t(p[2]) << 8) | 
    (uint32_t(p[3]));
}

//BE32S:把大端32位无符号读出来，再当成有符号int32_t
//这样才能正确得到负数（例如yaw可能得到负）
static int32_t be32s(const uint8_t* p){
    return (int32_t)be32(p);
}

int main() {
    /*
    这是一条完整帧（22字节），用于练习解析：
    预期解析结果：
      type=0x10
      len=7
      seq=4660
      pressure=101325
      temp=25.34C
      voltage=11650mV
      yaw=-12.3deg
    */
    const uint8_t frame[] = {
      0xAA,0x55,              // magic
      0x10,                   // type
      0x07,0x00,              // len (LE): 0x0007 -> 7
      0x34,0x12,0x00,0x00,     // seq (LE): 0x00001234 -> 4660
      0x00,0x01,0x8B,0xCD,     // pressure (BE): 0x00018BCD -> 101325
      0x09,0xE6,              // temp_x100 (BE): 0x09E6 -> 2534 -> 25.34℃
      0x82,0x2D,              // voltage (LE): 0x2D82 -> 11650
      0xFF,0xFF,0xFF,0x85,     // yaw_x10 (BE, signed): 0xFFFFFF85 -> -123 -> -12.3°
      0x0D                    // tail
    };

    // ---------- 1) 最基本的“帧合法性检查” ----------
    // 检查帧头 magic 和帧尾 tail 是否符合约定
    if (frame[0] != 0xAA || frame[1] != 0x55 || frame[21] != 0x0D) {
        std::cout << "bad frame\n";
        return 0;
    }

    // ---------- 2) 按帧格式的偏移，把字段逐个读出来 ----------
    // 1字节字段直接取数组元素即可
    uint8_t  type = frame[2];

    // 多字节字段不能直接*(uint16_t*)强转读取（可能非对齐、端序不一致）
    // 所以用上面写的 le16/be16/le32/be32 来“拼字节”
    uint16_t len  = le16(&frame[3]);       // 从偏移3开始读2字节，小端
    uint32_t seq  = le32(&frame[5]);       // 从偏移5开始读4字节，小端
    uint32_t pressure = be32(&frame[9]);   // 从偏移9开始读4字节，大端

    // 温度字段：2字节大端，但它是“有符号” int16_t
    // 先按 BE16 拼出来，再强转为 int16_t，让 0xFFxx 这种能表示负数
    int16_t  temp_x100 = (int16_t)be16(&frame[13]);

    uint16_t voltage = le16(&frame[15]);   // 2字节小端

    // yaw：4字节大端，且是有符号（可能为负）
    int32_t  yaw_x10 = be32s(&frame[17]);

    // ---------- 3) 打印（把定点数还原成浮点显示） ----------
    std::cout << "type=0x" << std::hex << int(type) << std::dec << "\n";
    std::cout << "len=" << len << "\n";
    std::cout << "seq=" << seq << "\n";
    std::cout << "pressure=" << pressure << " Pa\n";

    // temp_x100 / 100.0 ：把 “℃×100” 还原成 “℃”
    std::cout << "temp=" << (temp_x100 / 100.0) << " C\n";

    std::cout << "voltage=" << voltage << " mV\n";

    // yaw_x10 / 10.0 ：把 “deg×10” 还原成 “deg”
    std::cout << "yaw=" << (yaw_x10 / 10.0) << " deg\n";
}
