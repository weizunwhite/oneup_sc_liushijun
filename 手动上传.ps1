# ESP32 手动上传脚本
# 使用方法：在PowerShell中运行此脚本

Write-Host "===========================================" -ForegroundColor Cyan
Write-Host "  ESP32 固件手动上传工具" -ForegroundColor Cyan
Write-Host "===========================================" -ForegroundColor Cyan
Write-Host ""

# 固件路径
$firmware = "e:\up\uwbshubao\.pio\build\esp32dev\firmware.bin"
$bootloader = "e:\up\uwbshubao\.pio\build\esp32dev\bootloader.bin"
$partitions = "e:\up\uwbshubao\.pio\build\esp32dev\partitions.bin"
$boot_app0 = "$env:USERPROFILE\.platformio\packages\framework-arduinoespressif32\tools\partitions\boot_app0.bin"
$esptool = "C:\Users\weizu\.platformio\packages\tool-esptoolpy\esptool.py"

# 检查文件
if (-not (Test-Path $firmware)) {
    Write-Host "错误: 固件文件不存在!" -ForegroundColor Red
    Write-Host "请先编译: pio run" -ForegroundColor Yellow
    exit 1
}

Write-Host "固件文件: $firmware" -ForegroundColor Green
Write-Host ""

# 步骤提示
Write-Host "上传步骤:" -ForegroundColor Yellow
Write-Host "1. 确保ESP32通过USB连接到COM12" -ForegroundColor White
Write-Host "2. 按住ESP32的BOOT按钮" -ForegroundColor White
Write-Host "3. 按一下EN(RST)按钮，松开" -ForegroundColor White
Write-Host "4. 松开BOOT按钮" -ForegroundColor White
Write-Host "5. 按任意键开始上传..." -ForegroundColor White
Write-Host ""

$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")

Write-Host "开始上传..." -ForegroundColor Cyan

# 执行上传
python $esptool `
    --chip esp32 `
    --port COM4 `
    --baud 115200 `
    --before default_reset `
    --after hard_reset `
    write_flash -z `
    --flash_mode dio `
    --flash_freq 40m `
    --flash_size 4MB `
    0x1000 $bootloader `
    0x8000 $partitions `
    0xe000 $boot_app0 `
    0x10000 $firmware

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "===========================================" -ForegroundColor Green
    Write-Host "  上传成功！" -ForegroundColor Green
    Write-Host "===========================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "可以打开串口监视器查看输出（波特率115200）" -ForegroundColor Yellow
} else {
    Write-Host ""
    Write-Host "===========================================" -ForegroundColor Red
    Write-Host "  上传失败！" -ForegroundColor Red
    Write-Host "===========================================" -ForegroundColor Red
    Write-Host ""
    Write-Host "请重试以下操作：" -ForegroundColor Yellow
    Write-Host "1. 拔插USB线" -ForegroundColor White
    Write-Host "2. 重新进入下载模式（按BOOT+EN）" -ForegroundColor White
    Write-Host "3. 再次运行此脚本" -ForegroundColor White
}
