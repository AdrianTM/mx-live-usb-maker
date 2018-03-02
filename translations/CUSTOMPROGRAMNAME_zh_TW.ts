<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_TW">
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../mainwindow.ui" line="14"/>
        <source>Program_Name</source>
        <translation>Program_Name</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="33"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Select Target USB Device&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;選擇標的 USB 設備 Target USB Device&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="43"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Select ISO file&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;選擇 ISO 檔&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="88"/>
        <location filename="../mainwindow.cpp" line="446"/>
        <location filename="../mainwindow.cpp" line="464"/>
        <source>Select ISO</source>
        <translation>選擇 ISO 檔</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="100"/>
        <source>Refresh drive list</source>
        <translation>更新磁碟清單</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="112"/>
        <location filename="../mainwindow.cpp" line="393"/>
        <location filename="../mainwindow.cpp" line="399"/>
        <source>Show advanced options</source>
        <translation>顯示進階選項</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="130"/>
        <source>Advanced Options</source>
        <translation>進階選項</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="139"/>
        <source>Verbosity (less to more):</source>
        <translation>詳細程度（由低至高）：</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="190"/>
        <source>Size of ESP (uefi) partition:</source>
        <translation>ESP (uefi) 磁碟區大小：</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="222"/>
        <source>Make the ext4 filesystem even if one exists</source>
        <translation>即便目標上已有 ext4 檔案系統，仍然製作該檔案系統</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="229"/>
        <source>Save the original boot directory when updating a live-usb</source>
        <translation>更新 usb 直播版的時候，保存原本的 boot 目錄</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="236"/>
        <source>Use gpt partitioning instead of msdos</source>
        <translation>採用 gpt 磁碟區，不採用 msdos 磁碟區</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="239"/>
        <source>GPT partitioning</source>
        <translation>GPT 磁碟區</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="246"/>
        <source>Update (only update an existing live-usb)</source>
        <translation>更新檔案（只是更新既有的 usb 直播版）</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="253"/>
        <source>Don&apos;t replace syslinux files</source>
        <translation>不取代原本的 syslinux 檔案</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="256"/>
        <source>Keep syslinux files</source>
        <translation>保留 syslinux 檔</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="263"/>
        <source>Ignore USB/removable check</source>
        <translation>跳過不檢查 USB 與可抽取之設備</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="270"/>
        <source>Temporarily disable automounting</source>
        <translation>暫時凍結自動掛載功能</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="280"/>
        <source>Set pmbr_boot disk flag (won&apos;t boot via UEFI)</source>
        <translation>設定 pmbr_boot 磁碟旗標（無法透過 UEFI 開機）</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="293"/>
        <source>Options</source>
        <translation>選項</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="308"/>
        <source>Don&apos;t run commands that affect the usb device</source>
        <translation>一切會影響到 usb 設備的命令，均不執行</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="311"/>
        <source>Dry run (no change to system)</source>
        <translation>排演（不動到系統）</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="318"/>
        <source>clone from a mounted live-usb or iso-file.</source>
        <translation>從掛載在系統上的直播版 usb，或者 iso 檔，進行複製。</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="321"/>
        <source>Clone an existing live system</source>
        <translation>複製既有的直播系統</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="328"/>
        <source>Set up to boot from an encrypted partition, will prompt for pass phrase on first boot</source>
        <translation>進行設定，以從加密的磁碟區開機；首次開機時，會請你輸入密碼（pass phrase）</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="331"/>
        <source>Encrypt</source>
        <translation>加密</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="338"/>
        <source>Clone running live system</source>
        <translation>複製正在運行的直播系統</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="345"/>
        <source>Read-only, cannot be used with persistency</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="348"/>
        <source>Copy image to USB (dd)</source>
        <translation>將映像檔複製到 USB（dd）</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="371"/>
        <source>Percent of USB-device to use:</source>
        <translation>動用 USB 設備的多少百分比：</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="394"/>
        <source>Label ext partition:</source>
        <translation>ext 磁碟區的標籤：</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="442"/>
        <source>For distros other than antiX/MX use &quot;Copy image to USB (dd)&quot; only.</source>
        <translation>antiX/MX 以外的套件系統，只能使用「將映像檔複製到 USB（dd）」。</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="499"/>
        <source>Quit application</source>
        <translation>退出程式</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="502"/>
        <source>Close</source>
        <translation>關閉</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="510"/>
        <source>Alt+N</source>
        <translation>Alt+N</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="542"/>
        <source>Display help </source>
        <translation>顯示說明</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="545"/>
        <source>Help</source>
        <translation>說明</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="553"/>
        <source>Alt+H</source>
        <translation>Alt+H</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="569"/>
        <source>Back</source>
        <translation>上一步</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="628"/>
        <source>Next</source>
        <translation>下一步</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="655"/>
        <source>About this application</source>
        <translation>關於本程式</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="658"/>
        <source>About...</source>
        <translation>關於……</translation>
    </message>
    <message>
        <location filename="../mainwindow.ui" line="666"/>
        <source>Alt+B</source>
        <translation>Alt+B</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="72"/>
        <location filename="../mainwindow.cpp" line="239"/>
        <location filename="../mainwindow.cpp" line="379"/>
        <source>Failure</source>
        <translation>失敗</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="72"/>
        <source>Source and destination are on the same device, please select again.</source>
        <translation>讀取的來源與寫入的目標乃是同一個設備，請重新選擇。</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="94"/>
        <source>Writing %1 using &apos;dd&apos; command to /dev/%2,

Please wait until the the process is completed</source>
        <translation>正在利用「dd」命令將 %1 寫入 /dev/%2，

請等待程序執行完畢</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="237"/>
        <source>Success</source>
        <translation>成功</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="237"/>
        <source>LiveUSB creation successful!</source>
        <translation>直播版 USB 製作成功！</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="239"/>
        <source>Error encountered in the LiveUSB creation process</source>
        <translation>製作直播版 USB 的程序發生錯誤</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="285"/>
        <source>Error</source>
        <translation>錯誤</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="285"/>
        <source>Please select a USB device to write to</source>
        <translation>請選擇要寫入哪一個 USB 設備</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="288"/>
        <location filename="../mainwindow.cpp" line="458"/>
        <source>clone</source>
        <translation>複製</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="327"/>
        <source>About</source>
        <translation>關於</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="328"/>
        <source>Version: </source>
        <translation>版本：</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="329"/>
        <source>Program for creating a live-usb from an iso-file, another live-usb, a live-cd/dvd, or a running live system.</source>
        <translation>本程式可以製作直播版 usb，其資料來源可以是 iso 檔、另外一個直播版 usb、直播版 cd/dvd、正在運行的系統。</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="331"/>
        <source>Copyright (c) MX Linux</source>
        <translation>版權所有 (c) MX Linux</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="332"/>
        <location filename="../mainwindow.cpp" line="338"/>
        <source>License</source>
        <translation>授權條款</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="333"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="368"/>
        <source>Select an ISO file to write to the USB drive</source>
        <translation>選擇要用哪個 ISO 檔來寫入 USB 設備</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="374"/>
        <location filename="../mainwindow.cpp" line="442"/>
        <source>Select Source Directory</source>
        <translation>選擇資料來源目錄</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="379"/>
        <source>Could not find %1/antiX/linuxfs file</source>
        <translation>找不到 %1/antiX/linuxfs 檔案</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="395"/>
        <source>Hide advanced options</source>
        <translation>隱藏進階選項</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="441"/>
        <location filename="../mainwindow.cpp" line="456"/>
        <source>Select Source</source>
        <translation>選擇來源</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="445"/>
        <location filename="../mainwindow.cpp" line="462"/>
        <source>Select ISO file</source>
        <translation>選擇 ISO 檔</translation>
    </message>
</context>
<context>
    <name>QApplication</name>
    <message>
        <location filename="../main.cpp" line="52"/>
        <source>You must run this program as root.</source>
        <translation>本程式必須以 root 身份來執行。</translation>
    </message>
</context>
</TS>
