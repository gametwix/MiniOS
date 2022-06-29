# Module for system calls in MiniOS

Controls switching between user mode and kernel mode. This system operates on the basis of system interrupts and is presented as a system call.

## Mode switching control algorithm user and kernel (secure)

![](/images/alg.jpg)

### GDT table

<table>
<tr>
<td>

</td>
<td>
Entry number in gdt
</td>
<td>
Base
</td>
<td>
Limit
</td>
<td>
Access
</td>
<td>
Flags + Limit
</td>
</tr>

<tr>
<td>
Null segment
</td>
<td>
0
</td>
<td>
0
</td>
<td>
0
</td>
<td>
0
</td>
<td>
0
</td>
</tr>

<tr bgcolor="#cb4335">
<td>
Code segment
</td>
<td>
1
</td>
<td>
0
</td>
<td>
0xFFFFFFFF
</td>
<td>
0x9A
</td>
<td>
0xCF
</td>
</tr>

<tr bgcolor="#cb4335">
<td>
Data segment
</td>
<td>
2
</td>
<td>
0
</td>
<td>
0xFFFFFFFF
</td>
<td>
0x92
</td>
<td>
0xCF
</td>
</tr>

<tr bgcolor="#52be80">
<td>
User mode code segment
</td>
<td>
3
</td>
<td>
0
</td>
<td>
0xFFFFFFFF
</td>
<td>
0xFA
</td>
<td>
0xCF
</td>
</tr>

<tr bgcolor="#52be80">
<td>
User mode data segment
</td>
<td>
4
</td>
<td>
0
</td>
<td>
0xFFFFFFFF
</td>
<td>
0xF2
</td>
<td>
0xCF
</td>
</tr>

<tr>
<td>
TSS
</td>
<td>
5
</td>
<td>
*TSS
</td>
<td>
size of TSS
</td>
<td>
0x9A
</td>
<td>
0x00
</td>
</tr>


### Structure GDT