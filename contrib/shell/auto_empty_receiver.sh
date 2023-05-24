#!/bin/bash

##
# Generate an empty receiver file for the current Devcoin round.
# NOTE: Setup with crontab.
# WARNING: Do not run this script if the file custodians program has been
#          reinstated.
#
# Copyright (c) 2023 The Devcoin Developers
# License: MIT
# Author: Fernando Paredes Garcia <fernando@develcuy.com>
##

get_devcoin_round() {
  local indexinfo="$(devcoin-cli getindexinfo)"
  local block_height=$(echo "$indexinfo" | grep -oP '(?<="best_block_height": )\d+')
  local devcoin_round=$(( (block_height - 1) / 4000 + 1)) # Ceil of block_height/4000
  echo $devcoin_round
}
auto_empty_receiver() {
  local round=$(get_devcoin_round)
  local current_receiver_file=~/.devcoin/receiver/"receiver_$round.csv"

  if [[ ! -f "$current_receiver_file" ]]; then
    rm ~/.devcoin/receiver/write_next_when.txt # Force continue syncing
    touch $current_receiver_file
    cat <<EOF > $current_receiver_file
Format,pluribusunum
_beginpeers
https://receiver01.devcoin.org/receiver.csv
https://receiver02.devcoin.org/receiver/receiver.csv
https://receiver03.devcoin.org/receiver.csv
https://receiver04.devcoin.org/receiver.csv
_endpeers
_begincoins
_endcoins
EOF
  fi
}

if [[ -d ~/.devcoin/ ]]; then
  auto_empty_receiver
fi
