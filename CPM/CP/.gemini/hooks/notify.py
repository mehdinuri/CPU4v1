#!/usr/bin/env python3
import sys
import json
import subprocess


def notify(title, message):
    """Send a Windows balloon tip notification via powershell.exe (WSL -> Windows)."""
    ps_cmd = (
        "Add-Type -AssemblyName System.Windows.Forms; "
        "$n = New-Object System.Windows.Forms.NotifyIcon; "
        "$n.Icon = [System.Drawing.SystemIcons]::Information; "
        "$n.Visible = $true; "
        f"$n.ShowBalloonTip(6000, '{title}', '{message}', "
        "[System.Windows.Forms.ToolTipIcon]::Info); "
        "Start-Sleep -Milliseconds 500; "
        "$n.Dispose()"
    )
    subprocess.Popen(
        ["powershell.exe", "-Command", ps_cmd],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )


try:
    data = json.load(sys.stdin)
    notif_type = data.get("notification_type")

    if notif_type == "ToolPermission":
        notify("Gemini CLI", "Needs your attention")
    elif notif_type:
        notify("Gemini CLI", notif_type)
    else:
        # AfterAgent and other lifecycle events land here
        notify("Gemini CLI", "Task finished")

except Exception:
    pass

# Gemini requires valid JSON on stdout
print(json.dumps({}))
