#!/bin/bash

open_wechat_window_if_exists() {
  local notifier_items=$(
    dbus-send --session --print-reply \
      --dest=org.kde.StatusNotifierWatcher /StatusNotifierWatcher \
      org.freedesktop.DBus.Properties.GetAll string:org.kde.StatusNotifierWatcher |
      grep -o 'org.kde.StatusNotifierItem-[[:digit:]]-[[:digit:]]'
  )

  local notifier_item
  for notifier_item in $notifier_items; do
    local notifier_id=$(
      dbus-send --session --print-reply \
        --dest="${notifier_item/\// \/}" /StatusNotifierItem \
        org.freedesktop.DBus.Properties.Get string:org.kde.StatusNotifierItem string:Id
    )

    if [[ $notifier_id =~ "wechat" ]]; then
      echo "found wechat"

      dbus-send --session --print-reply \
        --dest="${notifier_item/\// \/}" /StatusNotifierItem \
        --type=method_call org.kde.StatusNotifierItem.Activate int32:0 int32:0
    fi
  done
}

open_wechat_window_if_exists

exec proot -b /app/wechat/libuosdevicea.so:/usr/lib/license/libuosdevicea.so \
  -b /app/license/etc/os-release:/etc/os-release \
  -b /app/license/etc/lsb-release:/etc/lsb-release \
  -b /app/license/var/lib/uos-license/.license.json:/var/lib/uos-license/.license.json \
  -b /app/license/var/uos/.license.key:/var/uos/.license.key \
  /app/wechat/wechat "$@"
