/* file:        vector.S
 * description: define a memory region we can put 16 8-byte pointers in
 *              (we'll only use 3 of them for now...)
 * Created
 *  Peter Desnoyers, Northeastern Khoury College, 2020
 * Modified
 *  Ji-Yong Shin, Northeastern Khoury College, 2021
 */
	/* defines a global symbol "vector" */
	.global	vector
	/* defines a section named ".vector" with allocatable (a), writable (w) flags.
         * this section contains data (@progbits) */
	.section .vector,"aw",@progbits 

vector:	.fill 16,8,0
