#!/bin/bash

get_wechat_notifier_item() {
  local notifier_items=$(
    gdbus call --session \
      --dest=org.kde.StatusNotifierWatcher --object-path /StatusNotifierWatcher \
      --method org.freedesktop.DBus.Properties.GetAll org.kde.StatusNotifierWatcher |
      grep -oE 'org.kde.StatusNotifierItem-[0-9]{1,}-[0-9]'
  )

  local notifier_item
  for notifier_item in $notifier_items; do
    local notifier_id=$(
      gdbus call --session \
        --dest="${notifier_item/\// \/}" --object-path /StatusNotifierItem \
        --method org.freedesktop.DBus.Properties.Get org.kde.StatusNotifierItem Id
    )

    if [[ $notifier_id =~ "wechat" ]]; then
      echo "${notifier_item/\// \/}"
    fi
  done
}

try_open_wechat_window() {
  local notifier_item=$(get_wechat_notifier_item)

  if [ -n "$notifier_item" ]; then
    gdbus call --session \
      --dest="$notifier_item" --object-path /StatusNotifierItem \
      --method org.kde.StatusNotifierItem.Activate 0 0 >/dev/null
  fi
}

try_exit_wechat() {
  local notifier_item=$(get_wechat_notifier_item)

  if [ -n "$notifier_item" ]; then
    gdbus call --session \
      --dest="$notifier_item" --object-path /MenuBar \
      --method com.canonical.dbusmenu.Event 1 clicked '<"">' 0 >/dev/null
  fi
}

if [ "$1" == "--exit-wechat" ]; then
  try_exit_wechat
  exit
fi

try_open_wechat_window

exec proot -b /app/wechat/libuosdevicea.so:/usr/lib/license/libuosdevicea.so \
  -b /app/license/etc/os-release:/etc/os-release \
  -b /app/license/etc/lsb-release:/etc/lsb-release \
  -b /app/license/var/lib/uos-license/.license.json:/var/lib/uos-license/.license.json \
  -b /app/license/var/uos/.license.key:/var/uos/.license.key \
  /app/wechat/wechat "$@"
