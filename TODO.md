# Userspace TODO

## Yocto

- Setup config repo (Mark)
- integrate
  [calsat-userspace](https://github.com/space-technologies-at-california/calsat-userspace) and
  [calsat-kernel-modules](https://github.com/space-technologies-at-california/calsat-kernel-modules)
  into the build (Mark)

## Init

- create a script to run all necessary processes at boot
- setup software watchdogs to restart processes or reboot device in the case of
  failure

## Orientation

- detumbling
- orienting towards the sun
- orienting towards the PCBSats
- orienting towards ground for downlink

## Communication Protocol

- receive commands sent from ground station
- place commands in queue or send to a separate scheduler (Jackson)
- collect and compress command output (Jackson)
- send result data back to the ground and verify successful delivery (Jackson)
- figure out how to send/receive commands with as little data as possible
  (Jackson)

## Deployment

- code to deploy PCBSats
	- waiting on motor specifications

## Meta Project

- Continuous integration (Mark)
- Setup test framework (Mark)

# Kernel TODO

## Linux RT

- look into Linux RT support (Mark)

## Drivers

- GPS driver (Mark)
- IMU drivers (Mark)
- Radio network driver (Mark)
- Telemetry driver (Mark)

# Electronics TODO

- update power supply (Mark)
- integrate radio and computer onto the PCB (Mark)
	- potentially add coin cell or faux coin cell line for RTC (Mark)
- verify GPS (Mark)
- add IMU (Mark)
- deployment switch (Mark)
