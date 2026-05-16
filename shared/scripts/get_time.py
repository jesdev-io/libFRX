import datetime
from datetime import timezone

Import("env")

utc_now = datetime.datetime.now(timezone.utc)
cet_now = utc_now + datetime.timedelta(hours=1)
timestamp = int(cet_now.timestamp())

if timestamp > 0xFFFFFFFF:
    timestamp = 0xFFFFFFFF

print(f"Setting FR2_LAST_FLASH_TIME to: {timestamp} (Unix timestamp of CET time)")
print(f"CET Time: {cet_now.year}-{cet_now.month:02d}-{cet_now.day:02d} {cet_now.hour:02d}:{cet_now.minute:02d}")

env.Append(
    CPPDEFINES=[
        ("FR2_LAST_FLASH_TIME", timestamp),
        ("FR2_LAST_FLASH_YEAR", cet_now.year),
        ("FR2_LAST_FLASH_MONTH", cet_now.month),
        ("FR2_LAST_FLASH_WDAY", cet_now.weekday()),
        ("FR2_LAST_FLASH_DAY", cet_now.day),
        ("FR2_LAST_FLASH_HOUR", cet_now.hour),
        ("FR2_LAST_FLASH_MINUTE", cet_now.minute)
    ]
)