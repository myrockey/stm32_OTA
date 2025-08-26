<?php
// === 配置区 ===============================================================
$firmwarePath = __DIR__ . '/firmware.bin';   // 固件绝对路径
$crcHeader    = 'X-Firmware-CRC';            // 自定义 CRC 头名，可按需改
// ==========================================================================

if (!is_file($firmwarePath)) {
    http_response_code(404);
    exit('Firmware not found.');
}

$fileSize = filesize($firmwarePath);
$fp       = fopen($firmwarePath, 'rb');
if (!$fp) {
    http_response_code(500);
    exit('Cannot open firmware.');
}

// ---------- CRC16 计算（按需启用，可缓存到文件旁） -------------------------
function crc16File($fp, $size)
{
    rewind($fp);
    $crc = 0xFFFF;
    while (!feof($fp) && $size > 0) {
        $chunk = fread($fp, min(4096, $size));
        $size -= strlen($chunk);
        for ($i = 0, $len = strlen($chunk); $i < $len; $i++) {
            $crc ^= ord($chunk[$i]);
            for ($j = 0; $j < 8; $j++) {
                $crc = ($crc & 1) ? ($crc >> 1) ^ 0xA001 : $crc >> 1;
            }
        }
    }
    return $crc;
}
$crc16 = crc16File($fp, $fileSize);
header("$crcHeader: " . sprintf('%04X', $crc16));

// ---------- Range 解析 ----------------------------------------------------
$rangeHeader = isset($_SERVER['HTTP_RANGE']) ? $_SERVER['HTTP_RANGE'] : '';
$start       = 0;
$end         = $fileSize - 1;
$isPartial   = false;

if ($rangeHeader && preg_match('/bytes=(\d+)-(\d*)/', $rangeHeader, $matches)) {
    $start = (int)$matches[1];
    $end   = ($matches[2] === '') ? $fileSize - 1 : (int)$matches[2];

    // 范围合法性检查
    if ($start > $end || $start >= $fileSize || $end >= $fileSize) {
        http_response_code(416); // Range Not Satisfiable
        header("Content-Range: bytes */$fileSize");
        exit;
    }
    $isPartial = true;
}

// ---------- 输出头部 -------------------------------------------------------
$length = $end - $start + 1;
if ($isPartial) {
    http_response_code(206);
    header("Content-Range: bytes $start-$end/$fileSize");
} else {
    http_response_code(200);
}
header('Content-Type: application/octet-stream');
header('Accept-Ranges: bytes');
header('Content-Length: ' . $length);

// ---------- 发送二进制分片 -------------------------------------------------
rewind($fp);
fseek($fp, $start);
$left = $length;
while ($left > 0 && !feof($fp)) {
    $chunk = fread($fp, min(8192, $left));
    echo $chunk;
    $left -= strlen($chunk);
}
fclose($fp);
exit;
?>