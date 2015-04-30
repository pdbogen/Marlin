Purpose / Application Layer
===========================

The master needs to do these things:
* Query for temperatures per stepper
* Configure stepper motor
** On/off per stepper
** Direction per stepper

(Actually stepping the motor is done with a one-wire hardware interrupt.)

Commands in general should not expect a response. We might have a "send 
temperatures" command, but should be prepared to receive a command ourselves 
telling us to update our state to match the new temperatures. (As opposed to 
doing "get temperature" and receiving a response.)

It might be possible to have proper bidirectional communication down the road, 
but we'll have to find an interrupt we can use to handle all of the serial 
communication, that way code can "block" while waiting for a response. (Any 
other paradigm would require too extensive modifications to Marlin, I'm afraid.)

Queuing / Transport Layer
=========================

Boards implement a small packet queue to facilitate resends. The packet at the 
head of the queue is sent any time now() > q->head->time+q->head->delay (and 
delay and count are increased slightly each time). Too many resends and the 
packet is dropped from the output queue (with an error sent to the debug port).

Asynchronously, when a valid ACK is received for that packet (matching by CRC), 
remove it from the queue and immediately send the next.

ACKs for other packets are silently discarded. Any packets with invalid CRCs are 
silently discarded.

Network-layer Protocol
======================

The network-layer utilizes packets as described below. Packets are checksummed, 
and invalid checksums are silently dropped. Valid checksums are immediately 
ACK'd before being processed.

ACK packets are not queued and are not resent. This means that if an ACK packet 
is lost or mangled, the command will be repeated. We should be careful to only 
implement commands that we don't mind repetition. (E.g., we shouldn't have a 
"toggle" command where a repeated toggle results in a noop.)

A future enhancement is the addition of sequence numbers, allowing is to drop 
resent packets within a reasonable window.

Packet Structure
----------------

  .------------------------------------.
  | Cmd (8bit) | Payload | CRC (16bit) |
  '------------------------------------'

Cmd is a one-byte command, usually an ASCII character.

Payload is a union of {long,unsigned long,float,uint16_t}. No type information 
is provided, it's expected that commands will know what to do.

CRC is the CRC16 of the preceding bytes.


Link-layer Protocol
===================

The link-layer is very simple, and uses fixed-length frames with unique sentinel 
values.

The link-layer encoding is as follows. Note that the actual number of bytes sent 
may be larger than the frame length, if bytes are ESC-encoded:

0x01 represents a SYN/SOH packet, indicating the start of the packet.

0x10 represents the ESC/DLE character, indicating that the following byte should 
be interpreted specially (see "escape sequences," below).

Escape Sequences
----------------

Escape   | Decoded value
---------|--------------
0x53 (S) | 0x01 (SYN/SOH)
0x45 (E) | 0x10 (ESC/DLE)
