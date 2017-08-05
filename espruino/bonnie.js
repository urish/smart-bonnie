Serial1.setup(9600, {tx: D12, rx: D11});

function playerCommand(cmd, arg1, arg2) {
  const data = [0x7e, 0xff, 0x6, cmd, 0, arg1, arg2, 0, 0, 0xef];
  const checksum = 0 - data[1] - data[2] - data[3] - data[4] - data[5] - data[6];
  data[7] = (checksum >> 8) & 0xff;
  data[8] = checksum & 0xff;
  Serial1.write(data);
}

playerCommand(0x6, 0, 30);
setTimeout(() => playerCommand(0x12, 0, 1), 10);

setInterval(() => {
  digitalWrite(D30, 0);
  setTimeout(() => digitalWrite(D30, 1), 50);
}, 1000);

setInterval(() => Serial1.available() && console.log(Serial1.read(Serial1.available())), 50);