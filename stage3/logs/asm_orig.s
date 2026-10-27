	.file	"asm_probe.cpp"
	.text
	.section	.text.unlikely,"x"
	.align 2
	.def	_ZNSt12_Vector_baseIjSaIjEED2Ev.isra.0;	.scl	3;	.type	32;	.endef
	.seh_proc	_ZNSt12_Vector_baseIjSaIjEED2Ev.isra.0
_ZNSt12_Vector_baseIjSaIjEED2Ev.isra.0:
.LFB1813:
	.seh_endprologue
	testq	%rcx, %rcx
	je	.L1
	subq	%rcx, %rdx
	jmp	_ZdlPvm
.L1:
	ret
	.seh_endproc
	.section	.text$_ZN3nms6SorterIj2NxNS_8PolylogTELNS_5OrderE0ELb0EE8next_runEm,"x"
	.linkonce discard
	.align 2
	.p2align 4
	.globl	_ZN3nms6SorterIj2NxNS_8PolylogTELNS_5OrderE0ELb0EE8next_runEm
	.def	_ZN3nms6SorterIj2NxNS_8PolylogTELNS_5OrderE0ELb0EE8next_runEm;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN3nms6SorterIj2NxNS_8PolylogTELNS_5OrderE0ELb0EE8next_runEm
_ZN3nms6SorterIj2NxNS_8PolylogTELNS_5OrderE0ELb0EE8next_runEm:
.LFB1610:
	pushq	%r14
	.seh_pushreg	%r14
	pushq	%r13
	.seh_pushreg	%r13
	pushq	%r12
	.seh_pushreg	%r12
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$32, %rsp
	.seh_stackalloc	32
	.seh_endprologue
	movl	$1, %edi
	movq	%rcx, %r12
	movq	8(%rdx), %rcx
	movq	%rdx, %r9
	movq	%r8, %rbp
	subq	%r8, %rcx
	cmpq	$1, %rcx
	je	.L5
	movq	(%rdx), %rax
	leaq	(%rax,%r8,4), %rbx
	movl	(%rbx), %edi
	cmpl	%edi, 4(%rbx)
	jnb	.L6
	cmpq	$2, %rcx
	jbe	.L30
	movl	4(%rax,%r8,4), %edx
	movl	$2, %edi
	leaq	8(%rbx), %rax
	jmp	.L10
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L32:
	addq	$1, %rdi
	addq	$4, %rax
	cmpq	%rdi, %rcx
	je	.L31
.L10:
	movl	%edx, %r8d
	movl	(%rax), %edx
	cmpl	%r8d, %edx
	jb	.L32
.L9:
	cmpq	%rax, %rbx
	je	.L14
	subq	$4, %rax
	cmpq	%rax, %rbx
	jnb	.L14
.L8:
	movq	%rbx, %rdx
	.p2align 5
	.p2align 4
	.p2align 3
.L12:
	movl	(%rdx), %r8d
	movl	(%rax), %r10d
	addq	$4, %rdx
	subq	$4, %rax
	movl	%r10d, -4(%rdx)
	movl	%r8d, 4(%rax)
	cmpq	%rax, %rdx
	jb	.L12
.L14:
	movq	104(%r9), %r13
	cmpq	%r13, %rdi
	jnb	.L5
	cmpq	%rcx, %r13
	cmova	%rcx, %r13
	cmpq	%r13, %rdi
	jnb	.L16
	.p2align 4
	.p2align 3
.L21:
	movl	(%rbx,%rdi,4), %esi
	movq	%rdi, %rcx
	xorl	%edx, %edx
	jmp	.L18
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L19:
	movq	%rcx, %rax
	subq	%rdx, %rax
	shrq	%rax
	addq	%rdx, %rax
	cmpl	(%rbx,%rax,4), %esi
	jnb	.L33
	movq	%rax, %rcx
.L18:
	cmpq	%rcx, %rdx
	jb	.L19
	leaq	0(,%rdx,4), %rax
	leaq	(%rbx,%rax), %r14
	cmpq	%rdi, %rdx
	jb	.L34
.L20:
	addq	$1, %rdi
	movl	%esi, (%r14)
	cmpq	%rdi, %r13
	jne	.L21
.L16:
	movq	%r13, %rdi
.L5:
	movq	%r12, %rax
	movq	%rbp, (%r12)
	movq	%rdi, 8(%r12)
	addq	$32, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	ret
.L6:
	cmpq	$2, %rcx
	jbe	.L13
	movl	4(%rax,%r8,4), %eax
	movl	$2, %edi
	jmp	.L15
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L35:
	addq	$1, %rdi
	cmpq	%rdi, %rcx
	je	.L14
.L15:
	movl	%eax, %edx
	movl	(%rbx,%rdi,4), %eax
	cmpl	%edx, %eax
	jnb	.L35
	jmp	.L14
	.p2align 4,,10
	.p2align 3
.L33:
	leaq	1(%rax), %rdx
	jmp	.L18
	.p2align 4,,10
	.p2align 3
.L34:
	movq	%rdi, %r8
	leaq	4(%rbx,%rax), %rcx
	subq	%rdx, %r8
	movq	%r14, %rdx
	leaq	0(,%r8,4), %r8
	call	memmove
	jmp	.L20
.L31:
	leaq	(%rbx,%rcx,4), %rax
	jmp	.L9
.L30:
	leaq	4(%rbx), %rax
	movl	$2, %edi
	jmp	.L8
.L13:
	cmpq	$3, 104(%rdx)
	movl	$2, %edi
	cmovnb	%rcx, %rdi
	jmp	.L5
	.seh_endproc
	.section	.text$_ZN3nms6SorterIj2NxNS_5SagTTILj64ELb1EEELNS_5OrderE0ELb0EE8next_runEm,"x"
	.linkonce discard
	.align 2
	.p2align 4
	.globl	_ZN3nms6SorterIj2NxNS_5SagTTILj64ELb1EEELNS_5OrderE0ELb0EE8next_runEm
	.def	_ZN3nms6SorterIj2NxNS_5SagTTILj64ELb1EEELNS_5OrderE0ELb0EE8next_runEm;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN3nms6SorterIj2NxNS_5SagTTILj64ELb1EEELNS_5OrderE0ELb0EE8next_runEm
_ZN3nms6SorterIj2NxNS_5SagTTILj64ELb1EEELNS_5OrderE0ELb0EE8next_runEm:
.LFB1612:
	pushq	%r14
	.seh_pushreg	%r14
	pushq	%r13
	.seh_pushreg	%r13
	pushq	%r12
	.seh_pushreg	%r12
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$32, %rsp
	.seh_stackalloc	32
	.seh_endprologue
	movl	$1, %edi
	movq	%rcx, %r12
	movq	8(%rdx), %rcx
	movq	%rdx, %r9
	movq	%r8, %rbp
	subq	%r8, %rcx
	cmpq	$1, %rcx
	je	.L37
	movq	(%rdx), %rax
	leaq	(%rax,%r8,4), %rbx
	movl	(%rbx), %edi
	cmpl	%edi, 4(%rbx)
	jnb	.L38
	cmpq	$2, %rcx
	jbe	.L62
	movl	4(%rax,%r8,4), %edx
	movl	$2, %edi
	leaq	8(%rbx), %rax
	jmp	.L42
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L64:
	addq	$1, %rdi
	addq	$4, %rax
	cmpq	%rdi, %rcx
	je	.L63
.L42:
	movl	%edx, %r8d
	movl	(%rax), %edx
	cmpl	%r8d, %edx
	jb	.L64
.L41:
	cmpq	%rax, %rbx
	je	.L46
	subq	$4, %rax
	cmpq	%rax, %rbx
	jnb	.L46
.L40:
	movq	%rbx, %rdx
	.p2align 5
	.p2align 4
	.p2align 3
.L44:
	movl	(%rdx), %r8d
	movl	(%rax), %r10d
	addq	$4, %rdx
	subq	$4, %rax
	movl	%r10d, -4(%rdx)
	movl	%r8d, 4(%rax)
	cmpq	%rax, %rdx
	jb	.L44
.L46:
	movq	104(%r9), %r13
	cmpq	%r13, %rdi
	jnb	.L37
	cmpq	%rcx, %r13
	cmova	%rcx, %r13
	cmpq	%r13, %rdi
	jnb	.L48
	.p2align 4
	.p2align 3
.L53:
	movl	(%rbx,%rdi,4), %esi
	movq	%rdi, %rcx
	xorl	%edx, %edx
	jmp	.L50
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L51:
	movq	%rcx, %rax
	subq	%rdx, %rax
	shrq	%rax
	addq	%rdx, %rax
	cmpl	(%rbx,%rax,4), %esi
	jnb	.L65
	movq	%rax, %rcx
.L50:
	cmpq	%rcx, %rdx
	jb	.L51
	leaq	0(,%rdx,4), %rax
	leaq	(%rbx,%rax), %r14
	cmpq	%rdi, %rdx
	jb	.L66
.L52:
	addq	$1, %rdi
	movl	%esi, (%r14)
	cmpq	%rdi, %r13
	jne	.L53
.L48:
	movq	%r13, %rdi
.L37:
	movq	%r12, %rax
	movq	%rbp, (%r12)
	movq	%rdi, 8(%r12)
	addq	$32, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	ret
.L38:
	cmpq	$2, %rcx
	jbe	.L45
	movl	4(%rax,%r8,4), %eax
	movl	$2, %edi
	jmp	.L47
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L67:
	addq	$1, %rdi
	cmpq	%rdi, %rcx
	je	.L46
.L47:
	movl	%eax, %edx
	movl	(%rbx,%rdi,4), %eax
	cmpl	%edx, %eax
	jnb	.L67
	jmp	.L46
	.p2align 4,,10
	.p2align 3
.L65:
	leaq	1(%rax), %rdx
	jmp	.L50
	.p2align 4,,10
	.p2align 3
.L66:
	movq	%rdi, %r8
	leaq	4(%rbx,%rax), %rcx
	subq	%rdx, %r8
	movq	%r14, %rdx
	leaq	0(,%r8,4), %r8
	call	memmove
	jmp	.L52
.L63:
	leaq	(%rbx,%rcx,4), %rax
	jmp	.L41
.L62:
	leaq	4(%rbx), %rax
	movl	$2, %edi
	jmp	.L40
.L45:
	cmpq	$3, 104(%rdx)
	movl	$2, %edi
	cmovnb	%rcx, %rdi
	jmp	.L37
	.seh_endproc
	.section .rdata,"dr"
.LC0:
	.ascii "vector::_M_default_append\0"
	.section	.text$_ZNSt6vectorIjSaIjEE17_M_default_appendEm,"x"
	.linkonce discard
	.align 2
	.p2align 4
	.globl	_ZNSt6vectorIjSaIjEE17_M_default_appendEm
	.def	_ZNSt6vectorIjSaIjEE17_M_default_appendEm;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZNSt6vectorIjSaIjEE17_M_default_appendEm
_ZNSt6vectorIjSaIjEE17_M_default_appendEm:
.LFB1742:
	pushq	%r15
	.seh_pushreg	%r15
	pushq	%r14
	.seh_pushreg	%r14
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$48, %rsp
	.seh_stackalloc	48
	.seh_endprologue
	movq	%rcx, %rbx
	movq	%rdx, %r9
	testq	%rdx, %rdx
	je	.L68
	movq	16(%rcx), %r10
	movq	8(%rcx), %r8
	movq	%r10, %rax
	subq	%r8, %rax
	sarq	$2, %rax
	cmpq	%rdx, %rax
	jb	.L70
	movl	$0, (%r8)
	leaq	4(%r8), %rcx
	subq	$1, %r9
	je	.L71
	leaq	(%rcx,%r9,4), %r9
	xorl	%edx, %edx
	movq	%r9, %rax
	movq	%r9, 32(%rsp)
	subq	%r8, %rax
	leaq	-4(%rax), %r8
	call	memset
	movq	32(%rsp), %rcx
.L71:
	movq	%rcx, 8(%rbx)
.L68:
	addq	$48, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%r14
	popq	%r15
	ret
	.p2align 4,,10
	.p2align 3
.L70:
	movq	(%rcx), %rax
	subq	%rax, %r8
	movq	%rax, %r14
	movabsq	$2305843009213693951, %rax
	movq	%r8, %rsi
	sarq	$2, %r8
	subq	%r8, %rax
	movq	%r8, %rdi
	cmpq	%rdx, %rax
	jb	.L91
	cmpq	%r8, %rdx
	movq	%r8, %rax
	movq	%rdx, 104(%rsp)
	cmovnb	%rdx, %rax
	movq	%r10, 32(%rsp)
	movabsq	$2305843009213693951, %rdx
	addq	%r8, %rax
	cmpq	%rdx, %rax
	cmova	%rdx, %rax
	leaq	0(,%rax,4), %rcx
	leaq	0(,%rax,4), %r15
	call	_Znwm
	movq	104(%rsp), %r9
	movq	32(%rsp), %r10
	movl	$0, (%rax,%rsi)
	movq	%rax, %r11
	movq	%r9, %rdx
	subq	$1, %rdx
	je	.L73
	leaq	0(,%rdx,4), %r8
	leaq	4(%rax,%rsi), %rcx
	xorl	%edx, %edx
	movq	%rax, 40(%rsp)
	call	memset
	movq	40(%rsp), %r11
	movq	104(%rsp), %r9
	movq	32(%rsp), %r10
.L73:
	movq	%rsi, %r8
	testq	%rsi, %rsi
	je	.L74
	movq	%r11, %rcx
	movq	%r14, %rdx
	movq	%r9, 104(%rsp)
	movq	%r10, 32(%rsp)
	call	memcpy
	movq	104(%rsp), %r9
	movq	32(%rsp), %r10
	movq	%rax, %r11
.L74:
	testq	%r14, %r14
	je	.L75
	movq	%r10, %rdx
	movq	%r14, %rcx
	movq	%r11, 32(%rsp)
	subq	%r14, %rdx
	movq	%r9, 104(%rsp)
	call	_ZdlPvm
	movq	32(%rsp), %r11
	movq	104(%rsp), %r9
.L75:
	addq	%rdi, %r9
	movq	%r11, (%rbx)
	leaq	(%r11,%r9,4), %rax
	addq	%r15, %r11
	movq	%rax, 8(%rbx)
	movq	%r11, 16(%rbx)
	addq	$48, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%r14
	popq	%r15
	ret
.L91:
	leaq	.LC0(%rip), %rcx
	call	_ZSt20__throw_length_errorPKc
	nop
	.seh_endproc
	.section	.text$_ZN3nms6SorterIj2NxNS_8PolylogTELNS_5OrderE0ELb0EE8merge_atEmmm,"x"
	.linkonce discard
	.align 2
	.p2align 4
	.globl	_ZN3nms6SorterIj2NxNS_8PolylogTELNS_5OrderE0ELb0EE8merge_atEmmm
	.def	_ZN3nms6SorterIj2NxNS_8PolylogTELNS_5OrderE0ELb0EE8merge_atEmmm;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN3nms6SorterIj2NxNS_8PolylogTELNS_5OrderE0ELb0EE8merge_atEmmm
_ZN3nms6SorterIj2NxNS_8PolylogTELNS_5OrderE0ELb0EE8merge_atEmmm:
.LFB1611:
	pushq	%r15
	.seh_pushreg	%r15
	pushq	%r14
	.seh_pushreg	%r14
	pushq	%r13
	.seh_pushreg	%r13
	pushq	%r12
	.seh_pushreg	%r12
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$104, %rsp
	.seh_stackalloc	104
	.seh_endprologue
	movq	(%rcx), %rax
	movq	%r8, 48(%rsp)
	leaq	(%rax,%rdx,4), %rax
	movq	%r8, %rdi
	movq	%rcx, %rsi
	leaq	0(,%r8,4), %r8
	movq	%rax, 56(%rsp)
	leaq	(%rax,%r8), %r13
	movq	%r9, 40(%rsp)
	movl	-4(%r13), %eax
	cmpl	%eax, 0(%r13)
	jnb	.L92
	movq	112(%rcx), %rcx
	movq	120(%rsi), %rax
	leaq	(%rdi,%r9), %rbx
	subq	%rcx, %rax
	sarq	$2, %rax
	cmpq	%rdi, %r9
	jb	.L95
	cmpq	%rdi, %rax
	jb	.L247
.L96:
	movq	56(%rsp), %rdx
	call	memcpy
	movq	112(%rsi), %rax
	xorl	%esi, %esi
	movq	%rax, 64(%rsp)
	cmpq	$1, %rbx
	jbe	.L98
	leaq	-1(%rbx), %rax
	bsrq	%rax, %rax
	leal	1(%rax), %esi
	movslq	%esi, %rsi
	imulq	%rsi, %rsi
.L98:
	movq	64(%rsp), %r14
	movq	56(%rsp), %r9
	movl	(%r14), %eax
	cmpl	%eax, 0(%r13)
	jnb	.L185
	movq	%r13, %r10
	xorl	%eax, %eax
	xorl	%edi, %edi
	xorl	%ebp, %ebp
	xorl	%r12d, %r12d
	.p2align 4
	.p2align 3
.L100:
	movq	40(%rsp), %rdx
	movq	%rsi, %r11
	movl	(%r14), %ecx
	leaq	4(%r13,%rax), %r8
	subq	$1, %rdx
	subq	%r12, %rdx
	cmpq	%rsi, %rdx
	cmovbe	%rdx, %r11
	testq	%r11, %r11
	je	.L120
	xorl	%ebx, %ebx
	jmp	.L122
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L249:
	cmpq	%rbx, %r11
	je	.L248
.L122:
	movl	(%r8,%rbx,4), %eax
	movq	%rbx, %r15
	addq	$1, %rbx
	cmpl	%ecx, %eax
	jb	.L249
	cmpq	$8, %rbx
	ja	.L124
.L179:
	addq	%rbx, %r12
	xorl	%eax, %eax
	.p2align 5
	.p2align 4
	.p2align 3
.L134:
	movl	(%r10,%rax,4), %edx
	movl	%edx, (%r9,%rax,4)
	addq	$1, %rax
	cmpq	%rbx, %rax
	jne	.L134
.L242:
	movq	56(%rsp), %rax
	addq	%rbx, %rdi
	leaq	(%rax,%rdi,4), %r9
.L133:
	cmpq	%r12, 40(%rsp)
	je	.L250
	movq	64(%rsp), %rcx
	leaq	0(,%rbp,4), %rax
	leaq	0(%r13,%r12,4), %rdx
	leaq	(%rcx,%rax), %r10
	movq	56(%rsp), %rcx
	leaq	(%rcx,%rdi,4), %r9
.L99:
	movl	(%rdx), %ecx
	movq	48(%rsp), %rdx
	movq	%rsi, %r11
	movq	64(%rsp), %rbx
	subq	$1, %rdx
	subq	%rbp, %rdx
	leaq	4(%rbx,%rax), %r8
	cmpq	%rsi, %rdx
	cmovbe	%rdx, %r11
	testq	%r11, %r11
	je	.L101
	xorl	%ebx, %ebx
	jmp	.L103
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L252:
	cmpq	%rbx, %r11
	je	.L251
.L103:
	movl	(%r8,%rbx,4), %eax
	movq	%rbx, %r14
	addq	$1, %rbx
	cmpl	%eax, %ecx
	jnb	.L252
	cmpq	$8, %rbx
	ja	.L107
.L181:
	addq	%rbx, %rbp
	xorl	%eax, %eax
	.p2align 5
	.p2align 4
	.p2align 3
.L117:
	movl	(%r10,%rax,4), %edx
	movl	%edx, (%r9,%rax,4)
	addq	$1, %rax
	cmpq	%rbx, %rax
	jne	.L117
.L241:
	addq	%rbx, %rdi
.L116:
	cmpq	%rbp, 48(%rsp)
	je	.L92
	movq	64(%rsp), %rax
	movq	56(%rsp), %rcx
	leaq	(%rax,%rbp,4), %r14
	leaq	0(,%r12,4), %rax
	leaq	0(%r13,%rax), %r10
	leaq	(%rcx,%rdi,4), %r9
	jmp	.L100
.L250:
	movq	48(%rsp), %r8
	subq	%rbp, %r8
	cmpq	$8, %r8
	ja	.L136
	testq	%r8, %r8
	je	.L92
	xorl	%eax, %eax
	.p2align 5
	.p2align 4
	.p2align 3
.L137:
	movl	(%r14,%rax,4), %edx
	movl	%edx, (%r9,%rax,4)
	addq	$1, %rax
	cmpq	%rax, %r8
	jne	.L137
.L92:
	addq	$104, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
	.p2align 4,,10
	.p2align 3
.L95:
	movq	40(%rsp), %rdi
	cmpq	%rdi, %rax
	jb	.L253
.L138:
	movq	40(%rsp), %rdi
	movq	%r13, %rdx
	leaq	0(,%rdi,4), %r8
	subq	$1, %rdi
	call	memcpy
	movq	56(%rsp), %rcx
	leaq	-1(%rbx), %rax
	movq	%rdi, 80(%rsp)
	leaq	(%rcx,%rax,4), %rdx
	movq	%rdx, 64(%rsp)
	movq	48(%rsp), %rdx
	subq	$1, %rdx
	leaq	(%rcx,%rdx,4), %rcx
	movq	%rdx, 72(%rsp)
	movq	112(%rsi), %rdx
	xorl	%esi, %esi
	movq	%rcx, 56(%rsp)
	leaq	(%rdx,%rdi,4), %r15
	cmpq	$1, %rbx
	jbe	.L140
	bsrq	%rax, %rax
	leal	1(%rax), %esi
	movslq	%esi, %rsi
	imulq	%rsi, %rsi
.L140:
	movq	56(%rsp), %r14
	movl	(%r14), %eax
	cmpl	%eax, (%r15)
	jb	.L254
	movq	64(%rsp), %r14
	movq	56(%rsp), %rax
	movq	%r15, %r11
	xorl	%ecx, %ecx
	xorl	%edi, %edi
	xorl	%ebp, %ebp
	xorl	%r12d, %r12d
	jmp	.L141
.L254:
	movq	64(%rsp), %r10
	movq	%r15, %r13
	xorl	%ecx, %ecx
	xorl	%edi, %edi
	xorl	%ebp, %ebp
	xorl	%r12d, %r12d
	.p2align 4
	.p2align 3
.L142:
	movq	72(%rsp), %rdx
	movq	%rsi, %r9
	movq	56(%rsp), %rbx
	movl	0(%r13), %eax
	subq	%r12, %rdx
	leaq	-4(%rbx,%rcx), %r11
	cmpq	%rsi, %rdx
	cmovbe	%rdx, %r9
	testq	%r9, %r9
	je	.L159
	movq	%r11, %rcx
	xorl	%ebx, %ebx
	jmp	.L161
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L256:
	subq	$4, %rcx
	cmpq	%rbx, %r9
	je	.L255
.L161:
	movq	%rbx, %r8
	addq	$1, %rbx
	cmpl	(%rcx), %eax
	jb	.L256
	cmpq	$8, %rbx
	ja	.L163
.L182:
	addq	%rbx, %r12
.L164:
	movq	%rbx, %rdx
	xorl	%eax, %eax
	negq	%rdx
	salq	$2, %rdx
	.p2align 5
	.p2align 4
	.p2align 3
.L173:
	movl	(%r14,%rax), %ecx
	movl	%ecx, (%r10,%rax)
	subq	$4, %rax
	cmpq	%rdx, %rax
	jne	.L173
.L244:
	addq	%rbx, %rdi
	movq	64(%rsp), %rcx
	movq	%rdi, %rax
	negq	%rax
	leaq	(%rcx,%rax,4), %r10
	cmpq	%r12, 48(%rsp)
	je	.L257
.L174:
	movq	56(%rsp), %rdx
	movq	%r12, %rax
	movq	%rbp, %rcx
	movq	64(%rsp), %rbx
	negq	%rax
	negq	%rcx
	leaq	(%rdx,%rax,4), %rax
	movq	%rdi, %rdx
	salq	$2, %rcx
	negq	%rdx
	leaq	(%r15,%rcx), %r11
	leaq	(%rbx,%rdx,4), %r14
.L141:
	movq	80(%rsp), %rdx
	movq	%rsi, %r9
	movl	(%rax), %eax
	leaq	-4(%r15,%rcx), %r10
	subq	%rbp, %rdx
	cmpq	%rsi, %rdx
	cmovbe	%rdx, %r9
	testq	%r9, %r9
	je	.L143
	movq	%r10, %rcx
	xorl	%ebx, %ebx
	jmp	.L145
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L259:
	subq	$4, %rcx
	cmpq	%rbx, %r9
	je	.L258
.L145:
	movq	%rbx, %r8
	addq	$1, %rbx
	cmpl	%eax, (%rcx)
	jnb	.L259
	cmpq	$8, %rbx
	ja	.L147
.L183:
	addq	%rbx, %rbp
.L148:
	movq	%rbx, %rdx
	xorl	%eax, %eax
	negq	%rdx
	salq	$2, %rdx
	.p2align 5
	.p2align 4
	.p2align 3
.L157:
	movl	(%r11,%rax), %ecx
	movl	%ecx, (%r14,%rax)
	subq	$4, %rax
	cmpq	%rax, %rdx
	jne	.L157
.L243:
	addq	%rbx, %rdi
.L156:
	cmpq	%rbp, 40(%rsp)
	je	.L92
	movq	%rbp, %rax
	movq	%r12, %rcx
	movq	64(%rsp), %rdx
	negq	%rax
	negq	%rcx
	leaq	(%r15,%rax,4), %r13
	movq	56(%rsp), %rax
	salq	$2, %rcx
	leaq	(%rax,%rcx), %r14
	movq	%rdi, %rax
	negq	%rax
	leaq	(%rdx,%rax,4), %r10
	jmp	.L142
.L185:
	movq	64(%rsp), %r10
	movq	%r13, %rdx
	xorl	%eax, %eax
	xorl	%edi, %edi
	xorl	%ebp, %ebp
	xorl	%r12d, %r12d
	jmp	.L99
	.p2align 4,,10
	.p2align 3
.L255:
	cmpq	%rbx, %rdx
	je	.L260
.L162:
	leaq	1(%rsi), %rcx
	movl	$1, %r8d
	cmpq	%rcx, %rdx
	movq	%rcx, %r9
	cmovbe	%rdx, %r9
	movq	%r9, %rbx
	negq	%rbx
	cmpl	4(%r11,%rbx,4), %eax
	jnb	.L261
	movq	%r15, 88(%rsp)
	jmp	.L165
	.p2align 6
	.p2align 4,,10
	.p2align 3
.L167:
	addq	%r8, %r8
	leaq	(%r8,%rsi), %rcx
	cmpq	%rcx, %rdx
	movq	%rcx, %rbx
	cmovbe	%rdx, %rbx
	movq	%rbx, %r15
	negq	%r15
	cmpl	4(%r11,%r15,4), %eax
	jnb	.L240
	movq	%rbx, %r9
.L165:
	cmpq	%rdx, %rcx
	jb	.L167
	movq	88(%rsp), %r15
	leaq	1(%rdx), %rbx
	movq	%rdx, %r8
.L168:
	cmpq	$8, %rbx
	ja	.L163
	testq	%rbx, %rbx
	jne	.L182
	cmpq	%r12, 48(%rsp)
	jne	.L174
.L257:
	movq	40(%rsp), %rcx
	subq	%rbp, %rcx
	cmpq	$8, %rcx
	ja	.L175
	testq	%rcx, %rcx
	je	.L92
	negq	%rcx
	xorl	%eax, %eax
	salq	$2, %rcx
	.p2align 5
	.p2align 4
	.p2align 3
.L176:
	movl	0(%r13,%rax), %edx
	movl	%edx, (%r10,%rax)
	subq	$4, %rax
	cmpq	%rax, %rcx
	jne	.L176
	jmp	.L92
	.p2align 4,,10
	.p2align 3
.L251:
	cmpq	%rbx, %rdx
	je	.L262
.L106:
	leaq	1(%rsi), %rax
	movl	$1, %r11d
	cmpq	%rax, %rdx
	movq	%rax, %r14
	cmovbe	%rdx, %r14
	cmpl	-4(%r8,%r14,4), %ecx
	jnb	.L109
	jmp	.L263
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L111:
	addq	%r11, %r11
	leaq	(%r11,%rsi), %rax
	cmpq	%rax, %rdx
	movq	%rax, %rbx
	cmovbe	%rdx, %rbx
	cmpl	-4(%r8,%rbx,4), %ecx
	jb	.L114
	movq	%rbx, %r14
.L109:
	cmpq	%rdx, %rax
	jb	.L111
	leaq	1(%rdx), %rbx
.L112:
	cmpq	$8, %rbx
	ja	.L107
.L264:
	testq	%rbx, %rbx
	jne	.L181
	jmp	.L116
	.p2align 4,,10
	.p2align 3
.L187:
	movq	%rax, %rbx
.L114:
	movq	%rbx, %rax
	subq	%r14, %rax
	cmpq	$1, %rax
	jbe	.L112
.L115:
	shrq	%rax
	addq	%r14, %rax
	cmpl	-4(%r8,%rax,4), %ecx
	jb	.L187
	movq	%rax, %r14
	movq	%rbx, %rax
	subq	%r14, %rax
	cmpq	$1, %rax
	ja	.L115
	cmpq	$8, %rbx
	jbe	.L264
	.p2align 4
	.p2align 3
.L107:
	leaq	0(,%rbx,4), %r8
	movq	%r10, %rdx
	movq	%r9, %rcx
	addq	%rbx, %rbp
	call	memmove
	jmp	.L241
	.p2align 4,,10
	.p2align 3
.L248:
	cmpq	%rbx, %rdx
	je	.L265
.L123:
	leaq	1(%rsi), %rax
	movl	$1, %r11d
	cmpq	%rax, %rdx
	movq	%rax, %r15
	cmovbe	%rdx, %r15
	cmpl	%ecx, -4(%r8,%r15,4)
	jb	.L126
	jmp	.L266
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L128:
	addq	%r11, %r11
	leaq	(%r11,%rsi), %rax
	cmpq	%rax, %rdx
	movq	%rax, %rbx
	cmovbe	%rdx, %rbx
	cmpl	%ecx, -4(%r8,%rbx,4)
	jnb	.L131
	movq	%rbx, %r15
.L126:
	cmpq	%rdx, %rax
	jb	.L128
	leaq	1(%rdx), %rbx
.L129:
	cmpq	$8, %rbx
	ja	.L124
.L267:
	testq	%rbx, %rbx
	jne	.L179
	jmp	.L133
	.p2align 4,,10
	.p2align 3
.L189:
	movq	%rax, %rbx
.L131:
	movq	%rbx, %rax
	subq	%r15, %rax
	cmpq	$1, %rax
	jbe	.L129
.L132:
	shrq	%rax
	addq	%r15, %rax
	cmpl	%ecx, -4(%r8,%rax,4)
	jnb	.L189
	movq	%rax, %r15
	movq	%rbx, %rax
	subq	%r15, %rax
	cmpq	$1, %rax
	ja	.L132
	cmpq	$8, %rbx
	jbe	.L267
	.p2align 4
	.p2align 3
.L124:
	leaq	0(,%rbx,4), %r8
	movq	%r10, %rdx
	movq	%r9, %rcx
	addq	%rbx, %r12
	call	memmove
	jmp	.L242
	.p2align 4,,10
	.p2align 3
.L258:
	cmpq	%rbx, %rdx
	je	.L268
.L146:
	leaq	1(%rsi), %rcx
	movq	%rdx, %r9
	movl	$1, %r8d
	cmpq	%rdx, %rcx
	cmovbe	%rcx, %r9
	movq	%r9, %rbx
	negq	%rbx
	cmpl	%eax, 4(%r10,%rbx,4)
	jnb	.L149
	jmp	.L269
	.p2align 6
	.p2align 4,,10
	.p2align 3
.L151:
	addq	%r8, %r8
	leaq	(%r8,%rsi), %rcx
	cmpq	%rcx, %rdx
	movq	%rcx, %rbx
	cmovbe	%rdx, %rbx
	movq	%rbx, %r13
	negq	%r13
	cmpl	%eax, 4(%r10,%r13,4)
	jb	.L154
	movq	%rbx, %r9
.L149:
	cmpq	%rdx, %rcx
	jb	.L151
	leaq	1(%rdx), %rbx
	movq	%rdx, %r8
	cmpq	$8, %rbx
	ja	.L147
.L271:
	testq	%rbx, %rbx
	jne	.L183
	jmp	.L156
	.p2align 4,,10
	.p2align 3
.L194:
	movq	%rdx, %rbx
.L154:
	movq	%rbx, %rdx
	subq	%r9, %rdx
	cmpq	$1, %rdx
	jbe	.L270
.L155:
	shrq	%rdx
	addq	%r9, %rdx
	movq	%rdx, %rcx
	negq	%rcx
	cmpl	%eax, 4(%r10,%rcx,4)
	jb	.L194
	movq	%rdx, %r9
	movq	%rbx, %rdx
	subq	%r9, %rdx
	cmpq	$1, %rdx
	ja	.L155
.L270:
	leaq	-1(%rbx), %r8
	cmpq	$8, %rbx
	jbe	.L271
.L147:
	negq	%r8
	addq	%rbx, %rbp
	salq	$2, %r8
	leaq	(%r11,%r8), %rdx
	leaq	(%r14,%r8), %rcx
	leaq	0(,%rbx,4), %r8
	call	memmove
	jmp	.L243
.L195:
	movq	%rbx, %r8
	movq	%rax, %rbx
	.p2align 4
	.p2align 3
.L163:
	negq	%r8
	addq	%rbx, %r12
	salq	$2, %r8
	leaq	(%r14,%r8), %rdx
	leaq	(%r10,%r8), %rcx
	leaq	0(,%rbx,4), %r8
	call	memmove
	jmp	.L244
	.p2align 4,,10
	.p2align 3
.L240:
	movq	88(%rsp), %r15
	.p2align 4
	.p2align 3
.L170:
	movq	%rbx, %rdx
	subq	%r9, %rdx
	cmpq	$1, %rdx
	jbe	.L272
.L171:
	shrq	%rdx
	addq	%r9, %rdx
	movq	%rdx, %rcx
	negq	%rcx
	cmpl	4(%r11,%rcx,4), %eax
	jnb	.L197
	movq	%rdx, %r9
	movq	%rbx, %rdx
	subq	%r9, %rdx
	cmpq	$1, %rdx
	ja	.L171
.L272:
	leaq	-1(%rbx), %r8
	jmp	.L168
	.p2align 4,,10
	.p2align 3
.L197:
	movq	%rdx, %rbx
	jmp	.L170
	.p2align 4,,10
	.p2align 3
.L262:
	leaq	2(%r14), %rbx
	cmpq	$8, %rbx
	jbe	.L181
	jmp	.L107
	.p2align 4,,10
	.p2align 3
.L268:
	leaq	2(%r8), %rax
	cmpq	$8, %rax
	ja	.L192
	addq	%rax, %rbp
	movq	%rax, %rbx
	jmp	.L148
	.p2align 4,,10
	.p2align 3
.L265:
	leaq	2(%r15), %rbx
	cmpq	$8, %rbx
	jbe	.L179
	jmp	.L124
	.p2align 4,,10
	.p2align 3
.L260:
	leaq	2(%r8), %rax
	cmpq	$8, %rax
	ja	.L195
	addq	%rax, %r12
	movq	%rax, %rbx
	jmp	.L164
.L159:
	testq	%rdx, %rdx
	jne	.L162
	addq	$1, %r12
	movl	$1, %ebx
	jmp	.L164
.L143:
	testq	%rdx, %rdx
	jne	.L146
	addq	$1, %rbp
	movl	$1, %ebx
	jmp	.L148
.L101:
	testq	%rdx, %rdx
	jne	.L106
	movl	(%r10), %eax
	addq	$1, %rbp
	movl	$1, %ebx
	movl	%eax, (%r9)
	jmp	.L241
.L120:
	testq	%rdx, %rdx
	jne	.L123
	movl	(%r10), %eax
	addq	$1, %r12
	movl	$1, %ebx
	movl	%eax, (%r9)
	jmp	.L242
.L247:
	movq	%rdi, %rdx
	leaq	112(%rsi), %rcx
	movq	%r8, 64(%rsp)
	subq	%rax, %rdx
	call	_ZNSt6vectorIjSaIjEE17_M_default_appendEm
	movq	32(%rsi), %rax
	movq	112(%rsi), %rcx
	movq	64(%rsp), %r8
	cmpq	%rdi, 48(%rax)
	jnb	.L96
	movq	%rdi, 48(%rax)
	jmp	.L96
.L253:
	movq	%rdi, %rdx
	leaq	112(%rsi), %rcx
	subq	%rax, %rdx
	call	_ZNSt6vectorIjSaIjEE17_M_default_appendEm
	movq	32(%rsi), %rax
	movq	112(%rsi), %rcx
	cmpq	%rdi, 48(%rax)
	jnb	.L138
	movq	%rdi, 48(%rax)
	jmp	.L138
.L192:
	movq	%rbx, %r8
	movq	%rax, %rbx
	jmp	.L147
.L263:
	movq	%r14, %rbx
	movq	%rsi, %r14
	jmp	.L114
.L266:
	movq	%r15, %rbx
	movq	%rsi, %r15
	jmp	.L131
.L261:
	movq	%r9, %rbx
	movq	%rsi, %r9
	jmp	.L170
.L269:
	movq	%r9, %rbx
	movq	%rsi, %r9
	jmp	.L154
.L175:
	subq	40(%rsp), %rbp
	leaq	0(,%rcx,4), %r8
	leaq	4(,%rbp,4), %rax
	leaq	0(%r13,%rax), %rdx
	leaq	(%rax,%r10), %rcx
.L245:
	addq	$104, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	jmp	memmove
.L136:
	salq	$2, %r8
	movq	%r14, %rdx
	movq	%r9, %rcx
	jmp	.L245
	.seh_endproc
	.section	.text$_ZN3nms6SorterIj2NxNS_8PolylogTELNS_5OrderE0ELb0EE9run_powerEv,"x"
	.linkonce discard
	.align 2
	.p2align 4
	.globl	_ZN3nms6SorterIj2NxNS_8PolylogTELNS_5OrderE0ELb0EE9run_powerEv
	.def	_ZN3nms6SorterIj2NxNS_8PolylogTELNS_5OrderE0ELb0EE9run_powerEv;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN3nms6SorterIj2NxNS_8PolylogTELNS_5OrderE0ELb0EE9run_powerEv
_ZN3nms6SorterIj2NxNS_8PolylogTELNS_5OrderE0ELb0EE9run_powerEv:
.LFB1583:
	pushq	%r15
	.seh_pushreg	%r15
	pushq	%r14
	.seh_pushreg	%r14
	pushq	%r13
	.seh_pushreg	%r13
	pushq	%r12
	.seh_pushreg	%r12
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$2664, %rsp
	.seh_stackalloc	2664
	.seh_endprologue
	xorl	%r8d, %r8d
	leaq	80(%rsp), %rax
	movq	%rcx, %rdx
	movq	%rcx, %rdi
	movq	%rax, %rcx
	movq	%rax, 64(%rsp)
	call	_ZN3nms6SorterIj2NxNS_8PolylogTELNS_5OrderE0ELb0EE8next_runEm
	movq	88(%rsp), %r9
	cmpq	8(%rdi), %r9
	jnb	.L273
	movq	%r9, 32(%rsp)
	leaq	96(%rsp), %rax
	movq	80(%rsp), %r14
	xorl	%r13d, %r13d
	movq	%rax, 72(%rsp)
	leaq	608(%rsp), %rsi
	movq	%r9, %rax
	.p2align 4
	.p2align 3
.L282:
	movq	32(%rsp), %rbx
	movq	64(%rsp), %rcx
	movq	%rdi, %rdx
	xorl	%ebp, %ebp
	movq	%rax, 48(%rsp)
	movq	%rbx, %r8
	call	_ZN3nms6SorterIj2NxNS_8PolylogTELNS_5OrderE0ELb0EE8next_runEm
	movq	80(%rsp), %rax
	movq	48(%rsp), %r12
	movq	8(%rdi), %r10
	movq	%rax, 40(%rsp)
	movq	88(%rsp), %rax
	leaq	(%r12,%r14,2), %rdx
	addq	%rax, %rbx
	leaq	(%rax,%r12), %r8
	movq	%rbx, 32(%rsp)
	addq	%rdx, %r8
	jmp	.L279
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L287:
	subq	%r10, %rdx
	subq	%r10, %r8
.L276:
	addq	%rdx, %rdx
	addq	%r8, %r8
.L279:
	addl	$1, %ebp
	cmpq	%r10, %rdx
	jnb	.L287
	cmpq	%r10, %r8
	jb	.L276
	testl	%r13d, %r13d
	jle	.L288
	movq	72(%rsp), %rcx
	leal	-1(%r13), %ebx
	movq	%rsi, 56(%rsp)
	movq	%rax, 48(%rsp)
	leaq	(%rcx,%rbx,4), %r15
	salq	$4, %rbx
	addq	%rsi, %rbx
	movl	%r13d, %esi
	jmp	.L280
	.p2align 4,,10
	.p2align 3
.L281:
	movq	8(%rbx), %r13
	movq	(%rbx), %r14
	movq	%r12, %r9
	movq	%rdi, %rcx
	subl	$1, %esi
	subq	$4, %r15
	subq	$16, %rbx
	movq	%r13, %r8
	movq	%r14, %rdx
	addq	%r13, %r12
	call	_ZN3nms6SorterIj2NxNS_8PolylogTELNS_5OrderE0ELb0EE8merge_atEmmm
	testl	%esi, %esi
	je	.L289
.L280:
	cmpl	%ebp, (%r15)
	jnb	.L281
	movslq	%esi, %rdx
	leal	1(%rsi), %r13d
	movq	48(%rsp), %rax
	movq	8(%rdi), %r10
	movq	56(%rsp), %rsi
.L278:
	movq	%rdx, %r8
	movl	%ebp, 96(%rsp,%rdx,4)
	salq	$4, %r8
	movq	%r14, 608(%rsp,%r8)
	movq	40(%rsp), %r14
	movq	%r12, 616(%rsp,%r8)
	cmpq	%r10, 32(%rsp)
	jb	.L282
	movq	%rax, %r9
	testl	%r13d, %r13d
	jle	.L273
	leal	-1(%r13), %eax
	leaq	592(%rsp), %rbp
	movq	%r9, %rsi
	salq	$4, %rax
	leaq	608(%rsp,%rax), %rbx
	.p2align 4
	.p2align 3
.L283:
	movq	8(%rbx), %r12
	movq	(%rbx), %rdx
	movq	%rsi, %r9
	movq	%rdi, %rcx
	subq	$16, %rbx
	movq	%r12, %r8
	addq	%r12, %rsi
	call	_ZN3nms6SorterIj2NxNS_8PolylogTELNS_5OrderE0ELb0EE8merge_atEmmm
	cmpq	%rbp, %rbx
	jne	.L283
.L273:
	addq	$2664, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
	.p2align 4,,10
	.p2align 3
.L289:
	movq	48(%rsp), %rax
	movq	56(%rsp), %rsi
	xorl	%edx, %edx
	movl	$1, %r13d
	movq	8(%rdi), %r10
	jmp	.L278
.L288:
	movslq	%r13d, %rdx
	addl	$1, %r13d
	jmp	.L278
	.seh_endproc
	.text
	.p2align 4
	.globl	_Z10probe_polyPjm
	.def	_Z10probe_polyPjm;	.scl	2;	.type	32;	.endef
	.seh_proc	_Z10probe_polyPjm
_Z10probe_polyPjm:
.LFB1526:
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$176, %rsp
	.seh_stackalloc	176
	.seh_endprologue
	pxor	%xmm0, %xmm0
	leaq	72(%rsp), %rax
	movq	%rcx, 32(%rsp)
	leaq	32(%rsp), %rcx
	movq	%rdx, 40(%rsp)
	movq	$0, 48(%rsp)
	movq	$0, 56(%rsp)
	movq	$0, 120(%rsp)
	movq	$0, 128(%rsp)
	movq	$32, 136(%rsp)
	movq	$0, 160(%rsp)
	movq	%rax, 64(%rsp)
	movups	%xmm0, 72(%rsp)
	movups	%xmm0, 88(%rsp)
	movups	%xmm0, 104(%rsp)
	movaps	%xmm0, 144(%rsp)
	cmpq	$1, %rdx
	jbe	.L290
.LEHB0:
	call	_ZN3nms6SorterIj2NxNS_8PolylogTELNS_5OrderE0ELb0EE9run_powerEv
.LEHE0:
	movq	144(%rsp), %rcx
	testq	%rcx, %rcx
	je	.L290
	movq	160(%rsp), %rdx
	subq	%rcx, %rdx
	call	_ZdlPvm
	nop
.L290:
	addq	$176, %rsp
	popq	%rbx
	ret
.L295:
	movq	144(%rsp), %rcx
	movq	160(%rsp), %rdx
	movq	%rax, %rbx
	call	_ZNSt12_Vector_baseIjSaIjEED2Ev.isra.0
	movq	%rbx, %rcx
.LEHB1:
	call	_Unwind_Resume
	nop
.LEHE1:
	.seh_handler	__gxx_personality_seh0, @unwind, @except
	.seh_handlerdata
.LLSDA1526:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE1526-.LLSDACSB1526
.LLSDACSB1526:
	.uleb128 .LEHB0-.LFB1526
	.uleb128 .LEHE0-.LEHB0
	.uleb128 .L295-.LFB1526
	.uleb128 0
	.uleb128 .LEHB1-.LFB1526
	.uleb128 .LEHE1-.LEHB1
	.uleb128 0
	.uleb128 0
.LLSDACSE1526:
	.text
	.seh_endproc
	.section	.text$_ZN3nms6SorterIj2NxNS_6NaiveTELNS_5OrderE1ELb0EE8merge_atEmmm,"x"
	.linkonce discard
	.align 2
	.p2align 4
	.globl	_ZN3nms6SorterIj2NxNS_6NaiveTELNS_5OrderE1ELb0EE8merge_atEmmm
	.def	_ZN3nms6SorterIj2NxNS_6NaiveTELNS_5OrderE1ELb0EE8merge_atEmmm;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN3nms6SorterIj2NxNS_6NaiveTELNS_5OrderE1ELb0EE8merge_atEmmm
_ZN3nms6SorterIj2NxNS_6NaiveTELNS_5OrderE1ELb0EE8merge_atEmmm:
.LFB1628:
	pushq	%r14
	.seh_pushreg	%r14
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$72, %rsp
	.seh_stackalloc	72
	.seh_endprologue
	movq	(%rcx), %rax
	movq	%r8, %rbx
	leaq	(%rax,%rdx,4), %rsi
	leaq	0(,%r8,4), %r8
	movq	%rcx, %rdi
	leaq	(%rsi,%r8), %r11
	movl	-4(%r11), %eax
	cmpl	%eax, (%r11)
	jnb	.L299
	movq	112(%rcx), %rcx
	movq	120(%rdi), %rax
	subq	%rcx, %rax
	sarq	$2, %rax
	cmpq	%rbx, %r9
	jb	.L302
	cmpq	%rbx, %rax
	jb	.L344
.L303:
	movq	%rsi, %rdx
	movq	%r9, 40(%rsp)
	movq	%r11, 48(%rsp)
	call	memcpy
	movq	40(%rsp), %r9
	movq	112(%rdi), %rdi
	xorl	%ecx, %ecx
	testq	%r9, %r9
	je	.L322
	testq	%rbx, %rbx
	je	.L322
	movq	48(%rsp), %r11
	xorl	%r8d, %r8d
	xorl	%edx, %edx
	jmp	.L308
	.p2align 4,,10
	.p2align 3
.L345:
	movl	%r10d, %eax
	addq	$1, %r8
	movl	%eax, -4(%rsi,%rcx,4)
	cmpq	%rbx, %rdx
	jnb	.L299
.L346:
	cmpq	%r9, %r8
	jnb	.L305
.L308:
	movl	(%rdi,%rdx,4), %eax
	movl	(%r11,%r8,4), %r10d
	addq	$1, %rcx
	cmpl	%eax, %r10d
	jb	.L345
	addq	$1, %rdx
	movl	%eax, -4(%rsi,%rcx,4)
	cmpq	%rbx, %rdx
	jb	.L346
.L299:
	addq	$72, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%r14
	ret
	.p2align 4,,10
	.p2align 3
.L302:
	cmpq	%r9, %rax
	jb	.L347
	leaq	0(,%r9,4), %r8
	movq	%r11, %rdx
	movq	%r9, 40(%rsp)
	call	memcpy
	movq	40(%rsp), %r9
	testq	%r9, %r9
	je	.L299
.L315:
	leaq	-1(%r9), %rax
	xorl	%r11d, %r11d
	xorl	%r8d, %r8d
	leaq	(%rbx,%rax), %rdx
	leaq	(%rsi,%rdx,4), %rcx
	movq	112(%rdi), %rdx
	leaq	-4(%rsi,%rbx,4), %rsi
	movq	%rcx, %r14
	leaq	(%rdx,%rax,4), %rdi
	movq	%rcx, %rdx
	xorl	%ecx, %ecx
	jmp	.L318
	.p2align 4,,10
	.p2align 3
.L349:
	addq	$1, %rcx
	movl	%r10d, %eax
.L317:
	movl	%eax, (%rdx)
	subq	$4, %rdx
	cmpq	%r9, %r8
	jnb	.L299
	cmpq	%rbx, %rcx
	jnb	.L348
.L318:
	movq	%rcx, %rax
	addq	$1, %r11
	negq	%rax
	movl	(%rsi,%rax,4), %r10d
	movq	%r8, %rax
	negq	%rax
	movl	(%rdi,%rax,4), %eax
	cmpl	%r10d, %eax
	jb	.L349
	addq	$1, %r8
	jmp	.L317
	.p2align 4,,10
	.p2align 3
.L322:
	xorl	%edx, %edx
	.p2align 4
	.p2align 3
.L305:
	cmpq	%rbx, %rdx
	jnb	.L299
	subq	%rdx, %rbx
	leaq	(%rsi,%rcx,4), %rcx
	leaq	(%rdi,%rdx,4), %rdx
	cmpq	$8, %rbx
	ja	.L311
	xorl	%eax, %eax
	.p2align 5
	.p2align 4
	.p2align 3
.L312:
	movl	(%rdx,%rax,4), %r8d
	movl	%r8d, (%rcx,%rax,4)
	addq	$1, %rax
	cmpq	%rax, %rbx
	jne	.L312
	addq	$72, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%r14
	ret
	.p2align 4,,10
	.p2align 3
.L348:
	cmpq	%r9, %r8
	jnb	.L299
	movq	%r9, %rax
	movq	%r8, %rdx
	negq	%r11
	negq	%rdx
	subq	%r8, %rax
	leaq	(%r14,%r11,4), %rcx
	leaq	(%rdi,%rdx,4), %rdx
	cmpq	$8, %rax
	ja	.L320
	negq	%rax
	leaq	0(,%rax,4), %r9
	xorl	%eax, %eax
	.p2align 5
	.p2align 4
	.p2align 3
.L321:
	movl	(%rdx,%rax), %r8d
	movl	%r8d, (%rcx,%rax)
	subq	$4, %rax
	cmpq	%rax, %r9
	jne	.L321
	addq	$72, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%r14
	ret
	.p2align 4,,10
	.p2align 3
.L344:
	movq	%rbx, %rdx
	leaq	112(%rdi), %rcx
	movq	%r9, 56(%rsp)
	subq	%rax, %rdx
	movq	%r11, 48(%rsp)
	movq	%r8, 40(%rsp)
	call	_ZNSt6vectorIjSaIjEE17_M_default_appendEm
	movq	32(%rdi), %rax
	movq	112(%rdi), %rcx
	movq	40(%rsp), %r8
	movq	48(%rsp), %r11
	cmpq	%rbx, 48(%rax)
	movq	56(%rsp), %r9
	jnb	.L303
	movq	%rbx, 48(%rax)
	jmp	.L303
	.p2align 4,,10
	.p2align 3
.L347:
	movq	%r9, %rdx
	leaq	112(%rdi), %rcx
	movq	%r9, 40(%rsp)
	subq	%rax, %rdx
	movq	%r11, 48(%rsp)
	call	_ZNSt6vectorIjSaIjEE17_M_default_appendEm
	movq	32(%rdi), %rax
	movq	112(%rdi), %rcx
	movq	40(%rsp), %r9
	cmpq	%r9, 48(%rax)
	jb	.L350
	movq	48(%rsp), %rdx
	leaq	0(,%r9,4), %r8
	movq	%r9, 40(%rsp)
	call	memcpy
	movq	40(%rsp), %r9
	jmp	.L315
	.p2align 4,,10
	.p2align 3
.L311:
	leaq	0(,%rbx,4), %r8
.L343:
	addq	$72, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%r14
	jmp	memmove
	.p2align 4,,10
	.p2align 3
.L350:
	movq	%r9, 48(%rax)
	movq	48(%rsp), %rdx
	leaq	0(,%r9,4), %r8
	call	memcpy
	movq	40(%rsp), %r9
	jmp	.L315
	.p2align 4,,10
	.p2align 3
.L320:
	subq	%r9, %r8
	leaq	4(,%r8,4), %r8
	addq	%r8, %rdx
	addq	%r8, %rcx
	leaq	0(,%rax,4), %r8
	jmp	.L343
	.seh_endproc
	.section .rdata,"dr"
.LC1:
	.ascii "vector::_M_realloc_append\0"
	.section	.text$_ZN3nms6SorterIj2NxNS_6NaiveTELNS_5OrderE1ELb0EE7run_timEv,"x"
	.linkonce discard
	.align 2
	.p2align 4
	.globl	_ZN3nms6SorterIj2NxNS_6NaiveTELNS_5OrderE1ELb0EE7run_timEv
	.def	_ZN3nms6SorterIj2NxNS_6NaiveTELNS_5OrderE1ELb0EE7run_timEv;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN3nms6SorterIj2NxNS_6NaiveTELNS_5OrderE1ELb0EE7run_timEv
_ZN3nms6SorterIj2NxNS_6NaiveTELNS_5OrderE1ELb0EE7run_timEv:
.LFB1585:
	pushq	%r15
	.seh_pushreg	%r15
	pushq	%r14
	.seh_pushreg	%r14
	pushq	%r13
	.seh_pushreg	%r13
	pushq	%r12
	.seh_pushreg	%r12
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$72, %rsp
	.seh_stackalloc	72
	.seh_endprologue
	xorl	%r14d, %r14d
	xorl	%ebp, %ebp
	xorl	%r12d, %r12d
	xorl	%r15d, %r15d
	movq	$0, 56(%rsp)
	movq	%rcx, %r13
	cmpq	$32, %r14
	jle	.L353
	.p2align 4
	.p2align 3
.L421:
	leaq	-48(%r12,%r14), %rax
	leaq	-16(%r12,%r14), %rdi
	movq	8(%rax), %r8
	movq	8(%rdi), %r9
	leaq	-32(%r12,%r14), %rsi
	movq	8(%rsi), %rbx
	cmpq	%r9, %r8
	jb	.L416
	cmpq	%rbx, %r9
	jnb	.L390
	leaq	(%rbx,%r9), %rdi
	cmpq	%r8, %rdi
	jnb	.L362
	cmpq	$48, %r14
	jle	.L359
	addq	%rbx, %r8
	cmpq	-56(%r12,%r14), %r8
	jnb	.L362
.L359:
	movq	8(%r13), %rdx
	cmpq	%rdx, %r15
	jnb	.L414
	subq	%r15, %rdx
	movl	$1, %edi
	cmpq	$1, %rdx
	je	.L365
	movq	0(%r13), %rcx
	leaq	0(,%r15,4), %rax
	leaq	(%rcx,%rax), %rbx
	movl	(%rbx), %edi
	cmpl	%edi, 4(%rbx)
	jnb	.L366
	cmpq	$2, %rdx
	je	.L417
	movl	4(%rcx,%rax), %ecx
	movl	$2, %edi
	leaq	8(%rbx), %rax
	jmp	.L370
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L419:
	addq	$1, %rdi
	addq	$4, %rax
	cmpq	%rdi, %rdx
	je	.L418
.L370:
	movl	%ecx, %r8d
	movl	(%rax), %ecx
	cmpl	%r8d, %ecx
	jb	.L419
.L369:
	cmpq	%rbx, %rax
	je	.L373
	subq	$4, %rax
	cmpq	%rax, %rbx
	jnb	.L373
.L368:
	movq	%rbx, %rcx
	.p2align 5
	.p2align 4
	.p2align 3
.L372:
	movl	(%rcx), %r8d
	movl	(%rax), %r9d
	addq	$4, %rcx
	subq	$4, %rax
	movl	%r9d, -4(%rcx)
	movl	%r8d, 4(%rax)
	cmpq	%rax, %rcx
	jb	.L372
.L373:
	movq	104(%r13), %r9
	cmpq	%r9, %rdi
	jb	.L420
.L365:
	cmpq	%rbp, 56(%rsp)
	je	.L381
.L425:
	movq	%r15, 0(%rbp)
	addq	$16, %rbp
	movq	%rdi, -8(%rbp)
.L382:
	addq	%rdi, %r15
.L357:
	movq	%rbp, %r14
	subq	%r12, %r14
	cmpq	$32, %r14
	jg	.L421
.L353:
	cmpq	$16, %r14
	jle	.L359
	leaq	-32(%r12,%r14), %rsi
	movq	24(%r12), %r9
	movq	8(%rsi), %rbx
	cmpq	%rbx, %r9
	jb	.L359
.L390:
	movq	(%rsi), %rdx
	movq	%rbx, %r8
	movq	%r13, %rcx
	movq	%r9, 32(%rsp)
.LEHB2:
	call	_ZN3nms6SorterIj2NxNS_6NaiveTELNS_5OrderE1ELb0EE8merge_atEmmm
	movq	32(%rsp), %r9
	addq	%rbx, %r9
	movq	%r9, 8(%rsi)
.L411:
	subq	$16, %rbp
	jmp	.L357
	.p2align 4,,10
	.p2align 3
.L386:
	leaq	-32(%r12,%rax), %rsi
	movq	-8(%r12,%rax), %rbx
	movq	%r13, %rcx
	movq	8(%rsi), %rdi
	movq	(%rsi), %rdx
	movq	%rbx, %r9
	movq	%rdi, %r8
	call	_ZN3nms6SorterIj2NxNS_6NaiveTELNS_5OrderE1ELb0EE8merge_atEmmm
.LEHE2:
	addq	%rdi, %rbx
	subq	$16, %rbp
	movq	%rbx, 8(%rsi)
.L414:
	movq	%rbp, %rax
	subq	%r12, %rax
	cmpq	$16, %rax
	jg	.L386
	testq	%r12, %r12
	je	.L351
	movq	56(%rsp), %rdx
	movq	%r12, %rcx
	subq	%r12, %rdx
	addq	$72, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
.LEHB3:
	jmp	_ZdlPvm
.LEHE3:
	.p2align 4,,10
	.p2align 3
.L362:
	movq	(%rsi), %rdx
	movq	%rbx, %r8
	movq	%r13, %rcx
.LEHB4:
	call	_ZN3nms6SorterIj2NxNS_6NaiveTELNS_5OrderE1ELb0EE8merge_atEmmm
	movq	%rdi, 8(%rsi)
	subq	$16, %rbp
	jmp	.L357
.L366:
	cmpq	$2, %rdx
	je	.L393
	movl	4(%rcx,%rax), %eax
	movl	$2, %edi
	jmp	.L374
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L422:
	addq	$1, %rdi
	cmpq	%rdi, %rdx
	je	.L373
.L374:
	movl	%eax, %ecx
	movl	(%rbx,%rdi,4), %eax
	cmpl	%ecx, %eax
	jnb	.L422
	movq	104(%r13), %r9
	cmpq	%r9, %rdi
	jnb	.L365
.L420:
	cmpq	%rdx, %r9
	cmova	%rdx, %r9
	cmpq	%r9, %rdi
	jnb	.L375
	.p2align 4
	.p2align 3
.L380:
	movl	(%rbx,%rdi,4), %esi
	movq	%rdi, %rdx
	xorl	%ecx, %ecx
	jmp	.L377
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L378:
	movq	%rdx, %rax
	subq	%rcx, %rax
	shrq	%rax
	addq	%rcx, %rax
	cmpl	(%rbx,%rax,4), %esi
	jnb	.L423
	movq	%rax, %rdx
.L377:
	cmpq	%rdx, %rcx
	jb	.L378
	leaq	0(,%rcx,4), %r8
	leaq	(%rbx,%r8), %rdx
	cmpq	%rdi, %rcx
	jb	.L424
.L379:
	addq	$1, %rdi
	movl	%esi, (%rdx)
	cmpq	%rdi, %r9
	jne	.L380
.L375:
	movq	%r9, %rdi
	cmpq	%rbp, 56(%rsp)
	jne	.L425
.L381:
	movabsq	$9223372036854775792, %rsi
	movq	%r14, %rax
	sarq	$4, %rax
	cmpq	%rsi, %r14
	je	.L426
	testq	%rax, %rax
	movl	$1, %ebx
	movq	%r15, 32(%rsp)
	cmovne	%rax, %rbx
	movq	%rdi, 40(%rsp)
	addq	%rax, %rbx
	movabsq	$576460752303423487, %rax
	cmpq	%rax, %rbx
	cmova	%rax, %rbx
	salq	$4, %rbx
	movq	%rbx, %rcx
	call	_Znwm
	movq	32(%rsp), %r8
	movq	40(%rsp), %r9
	movq	%rax, %rsi
	movq	%r8, (%rax,%r14)
	movq	%r9, 8(%rax,%r14)
	testq	%r14, %r14
	je	.L384
	movq	%r14, %r8
	movq	%r12, %rdx
	movq	%rax, %rcx
	call	memcpy
.L384:
	leaq	16(%rsi,%r14), %rbp
	testq	%r12, %r12
	je	.L385
	movq	56(%rsp), %rdx
	movq	%r12, %rcx
	subq	%r12, %rdx
	call	_ZdlPvm
.L385:
	leaq	(%rsi,%rbx), %rax
	movq	%rsi, %r12
	movq	%rax, 56(%rsp)
	jmp	.L382
.L416:
	movq	(%rax), %rdx
	movq	%rbx, %r9
	movq	%r13, %rcx
	movq	%rax, 48(%rsp)
	movq	%r8, 32(%rsp)
	call	_ZN3nms6SorterIj2NxNS_6NaiveTELNS_5OrderE1ELb0EE8merge_atEmmm
	movq	32(%rsp), %r8
	movq	48(%rsp), %rax
	addq	%rbx, %r8
	movq	%r8, 8(%rax)
	cmpq	%rbp, %rdi
	je	.L411
	movq	%rbp, %r8
	subq	%rdi, %r8
	cmpq	$16, %r8
	jle	.L356
	movq	%rdi, %rdx
	movq	%rsi, %rcx
	call	memmove
	jmp	.L411
	.p2align 4,,10
	.p2align 3
.L423:
	leaq	1(%rax), %rcx
	jmp	.L377
	.p2align 4,,10
	.p2align 3
.L424:
	movq	%rdi, %rax
	movq	%r9, 48(%rsp)
	subq	%rcx, %rax
	leaq	4(%rbx,%r8), %rcx
	movq	%rdx, 32(%rsp)
	leaq	0(,%rax,4), %r8
	call	memmove
	movq	48(%rsp), %r9
	movq	32(%rsp), %rdx
	jmp	.L379
.L418:
	leaq	(%rbx,%rdx,4), %rax
	jmp	.L369
.L351:
	addq	$72, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
.L417:
	leaq	4(%rbx), %rax
	movl	$2, %edi
	jmp	.L368
.L393:
	movl	$2, %edi
	jmp	.L365
.L356:
	jne	.L411
	movdqu	16(%rsi), %xmm0
	movups	%xmm0, (%rsi)
	jmp	.L411
.L426:
	leaq	.LC1(%rip), %rcx
	call	_ZSt20__throw_length_errorPKc
.LEHE4:
.L395:
	movq	%rax, %rbx
	testq	%r12, %r12
	je	.L389
	movq	56(%rsp), %rdx
	movq	%r12, %rcx
	subq	%r12, %rdx
	call	_ZdlPvm
.L389:
	movq	%rbx, %rcx
.LEHB5:
	call	_Unwind_Resume
	nop
.LEHE5:
	.seh_handler	__gxx_personality_seh0, @unwind, @except
	.seh_handlerdata
.LLSDA1585:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE1585-.LLSDACSB1585
.LLSDACSB1585:
	.uleb128 .LEHB2-.LFB1585
	.uleb128 .LEHE2-.LEHB2
	.uleb128 .L395-.LFB1585
	.uleb128 0
	.uleb128 .LEHB3-.LFB1585
	.uleb128 .LEHE3-.LEHB3
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB4-.LFB1585
	.uleb128 .LEHE4-.LEHB4
	.uleb128 .L395-.LFB1585
	.uleb128 0
	.uleb128 .LEHB5-.LFB1585
	.uleb128 .LEHE5-.LEHB5
	.uleb128 0
	.uleb128 0
.LLSDACSE1585:
	.section	.text$_ZN3nms6SorterIj2NxNS_6NaiveTELNS_5OrderE1ELb0EE7run_timEv,"x"
	.linkonce discard
	.seh_endproc
	.text
	.p2align 4
	.globl	_Z11probe_naivePjm
	.def	_Z11probe_naivePjm;	.scl	2;	.type	32;	.endef
	.seh_proc	_Z11probe_naivePjm
_Z11probe_naivePjm:
.LFB1528:
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$176, %rsp
	.seh_stackalloc	176
	.seh_endprologue
	pxor	%xmm0, %xmm0
	leaq	72(%rsp), %rax
	movq	%rcx, 32(%rsp)
	leaq	32(%rsp), %rcx
	movq	%rdx, 40(%rsp)
	movq	$0, 48(%rsp)
	movq	$0, 56(%rsp)
	movq	$0, 120(%rsp)
	movq	$0, 128(%rsp)
	movq	$32, 136(%rsp)
	movq	$0, 160(%rsp)
	movq	%rax, 64(%rsp)
	movups	%xmm0, 72(%rsp)
	movups	%xmm0, 88(%rsp)
	movups	%xmm0, 104(%rsp)
	movaps	%xmm0, 144(%rsp)
	cmpq	$1, %rdx
	jbe	.L427
.LEHB6:
	call	_ZN3nms6SorterIj2NxNS_6NaiveTELNS_5OrderE1ELb0EE7run_timEv
.LEHE6:
	movq	144(%rsp), %rcx
	testq	%rcx, %rcx
	je	.L427
	movq	160(%rsp), %rdx
	subq	%rcx, %rdx
	call	_ZdlPvm
	nop
.L427:
	addq	$176, %rsp
	popq	%rbx
	ret
.L432:
	movq	144(%rsp), %rcx
	movq	160(%rsp), %rdx
	movq	%rax, %rbx
	call	_ZNSt12_Vector_baseIjSaIjEED2Ev.isra.0
	movq	%rbx, %rcx
.LEHB7:
	call	_Unwind_Resume
	nop
.LEHE7:
	.seh_handler	__gxx_personality_seh0, @unwind, @except
	.seh_handlerdata
.LLSDA1528:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE1528-.LLSDACSB1528
.LLSDACSB1528:
	.uleb128 .LEHB6-.LFB1528
	.uleb128 .LEHE6-.LEHB6
	.uleb128 .L432-.LFB1528
	.uleb128 0
	.uleb128 .LEHB7-.LFB1528
	.uleb128 .LEHE7-.LEHB7
	.uleb128 0
	.uleb128 0
.LLSDACSE1528:
	.text
	.seh_endproc
	.section	.text$_ZN3nms5SagTTILj64ELb1EE5adaptEmmmmm,"x"
	.linkonce discard
	.p2align 4
	.globl	_ZN3nms5SagTTILj64ELb1EE5adaptEmmmmm
	.def	_ZN3nms5SagTTILj64ELb1EE5adaptEmmmmm;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN3nms5SagTTILj64ELb1EE5adaptEmmmmm
_ZN3nms5SagTTILj64ELb1EE5adaptEmmmmm:
.LFB1747:
	pushq	%r12
	.seh_pushreg	%r12
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	.seh_endprologue
	movq	80(%rsp), %rax
	subq	$1, %rax
	movq	%rcx, %r11
	movq	%rcx, %rbx
	leaq	-1(%r8), %rbp
	movq	%rdx, %rdi
	shrq	%r11
	cmpq	%rcx, %r11
	je	.L437
	cmpq	%r11, %rax
	movq	%r11, %rcx
	cmovbe	%rax, %rcx
	cmpq	%rcx, %rbp
	jb	.L466
	cmpq	%rax, %r11
	jnb	.L438
	leaq	1(%r11), %rdx
	movl	$1, %esi
	cmpq	%rdx, %rax
	movq	%rdx, %r12
	cmovbe	%rax, %r12
	addq	$1, %rcx
	cmpq	%r8, %r12
	jb	.L439
	jmp	.L522
	.p2align 6
	.p2align 4,,10
	.p2align 3
.L523:
	addq	%rsi, %rsi
	leaq	(%r11,%rsi), %rdx
	cmpq	%rdx, %rax
	movq	%rdx, %r10
	cmovbe	%rax, %r10
	addq	$1, %rcx
	cmpq	%r8, %r10
	jnb	.L441
	movq	%r10, %r12
.L439:
	cmpq	%rax, %rdx
	jb	.L523
.L438:
	cmpq	%r9, %rcx
	jnb	.L524
.L469:
	movq	%rcx, %r9
.L437:
	leaq	2(%rbx,%rbx), %rsi
	cmpq	%rdi, %rsi
	cmova	%rdi, %rsi
	cmpq	%rbx, %rsi
	je	.L443
	cmpq	%rsi, %rax
	movq	%rsi, %rcx
	cmovbe	%rax, %rcx
	cmpq	%rcx, %rbp
	jb	.L444
	cmpq	%rax, %rsi
	jnb	.L445
	leaq	1(%rsi), %rdx
	movl	$1, %edi
	cmpq	%rdx, %rax
	movq	%rdx, %rbp
	cmovbe	%rax, %rbp
	addq	$1, %rcx
	cmpq	%r8, %rbp
	jb	.L446
	jmp	.L525
	.p2align 6
	.p2align 4,,10
	.p2align 3
.L526:
	addq	%rdi, %rdi
	leaq	(%rsi,%rdi), %rdx
	cmpq	%rdx, %rax
	movq	%rdx, %r10
	cmovbe	%rax, %r10
	addq	$1, %rcx
	cmpq	%r8, %r10
	jnb	.L450
	movq	%r10, %rbp
.L446:
	cmpq	%rax, %rdx
	jb	.L526
.L449:
	cmpq	%r9, %rcx
	jb	.L458
	testq	%rbx, %rbx
	je	.L436
.L456:
	cmpq	$1, %r8
	jbe	.L527
.L460:
	movl	$1, %r10d
	movl	$1, %ebx
	movl	$1, %edx
	jmp	.L463
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L528:
	addq	%rdx, %rdx
	cmpq	%rdx, %rax
	movq	%rdx, %rcx
	cmovbe	%rax, %rcx
	addq	$1, %r10
	cmpq	%r8, %rcx
	jnb	.L462
	movq	%rcx, %rbx
.L463:
	cmpq	%rax, %rdx
	jb	.L528
.L459:
	xorl	%eax, %eax
	cmpq	%r9, %r10
	cmovb	%rax, %r11
.L436:
	movq	%r11, %rax
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	ret
	.p2align 4,,10
	.p2align 3
.L524:
	movq	%rbx, %r11
	jmp	.L437
	.p2align 4,,10
	.p2align 3
.L444:
	cmpq	%r9, %r8
	jb	.L529
.L443:
	testq	%rbx, %rbx
	je	.L436
	xorl	%r10d, %r10d
	testq	%rax, %rax
	jne	.L456
	jmp	.L459
	.p2align 4,,10
	.p2align 3
.L462:
	movq	%rcx, %rax
	subq	%rbx, %rax
	cmpq	$1, %rax
	ja	.L461
	jmp	.L459
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L530:
	movq	%rcx, %rax
	movq	%rdx, %rbx
	subq	%rdx, %rax
.L464:
	cmpq	$1, %rax
	jbe	.L459
.L461:
	shrq	%rax
	addq	$1, %r10
	leaq	(%rax,%rbx), %rdx
	cmpq	%r8, %rdx
	jb	.L530
	movq	%rdx, %rcx
	jmp	.L464
	.p2align 4,,10
	.p2align 3
.L466:
	movq	%r8, %rcx
	cmpq	%r9, %rcx
	jb	.L469
	jmp	.L524
.L522:
	movq	%r12, %r10
	movq	%r11, %r12
	.p2align 4
	.p2align 3
.L441:
	movq	%r10, %rdx
	subq	%r12, %rdx
	cmpq	$1, %rdx
	jbe	.L438
	shrq	%rdx
	addq	$1, %rcx
	leaq	(%r12,%rdx), %rsi
	cmpq	%r8, %rsi
	jnb	.L468
	.p2align 4
	.p2align 3
.L531:
	movq	%r10, %rdx
	movq	%rsi, %r12
	subq	%rsi, %rdx
.L442:
	cmpq	$1, %rdx
	jbe	.L438
	shrq	%rdx
	addq	$1, %rcx
	leaq	(%r12,%rdx), %rsi
	cmpq	%r8, %rsi
	jb	.L531
.L468:
	movq	%rsi, %r10
	jmp	.L442
.L525:
	movq	%rbp, %r10
	movq	%rsi, %rbp
	.p2align 4
	.p2align 3
.L450:
	movq	%r10, %rdx
	subq	%rbp, %rdx
	cmpq	$1, %rdx
	jbe	.L449
	shrq	%rdx
	addq	$1, %rcx
	leaq	(%rdx,%rbp), %rdi
	cmpq	%r8, %rdi
	jnb	.L471
	.p2align 4
	.p2align 3
.L532:
	movq	%r10, %rdx
	movq	%rdi, %rbp
	subq	%rdi, %rdx
.L451:
	cmpq	$1, %rdx
	jbe	.L449
	shrq	%rdx
	addq	$1, %rcx
	leaq	(%rdx,%rbp), %rdi
	cmpq	%r8, %rdi
	jb	.L532
.L471:
	movq	%rdi, %r10
	jmp	.L451
	.p2align 4,,10
	.p2align 3
.L458:
	movq	%rsi, %r11
	movq	%rcx, %r9
	testq	%rbx, %rbx
	jne	.L456
	jmp	.L436
	.p2align 4,,10
	.p2align 3
.L529:
	movq	%rsi, %r11
	testq	%rbx, %rbx
	je	.L436
	xorl	%r11d, %r11d
	testq	%rax, %rax
	je	.L436
	cmpq	$1, %r8
	je	.L482
	movq	%r8, %r9
	movq	%rsi, %r11
	jmp	.L460
	.p2align 4,,10
	.p2align 3
.L445:
	cmpq	%r9, %rcx
	cmovb	%rcx, %r9
	cmovb	%rsi, %r11
	jmp	.L443
.L527:
	movl	$1, %r10d
	jmp	.L459
.L482:
	movq	%rsi, %r11
	jmp	.L436
	.seh_endproc
	.section	.text$_ZN3nms6SorterIj2NxNS_5SagTTILj64ELb1EEELNS_5OrderE0ELb0EE8merge_atEmmm,"x"
	.linkonce discard
	.align 2
	.p2align 4
	.globl	_ZN3nms6SorterIj2NxNS_5SagTTILj64ELb1EEELNS_5OrderE0ELb0EE8merge_atEmmm
	.def	_ZN3nms6SorterIj2NxNS_5SagTTILj64ELb1EEELNS_5OrderE0ELb0EE8merge_atEmmm;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN3nms6SorterIj2NxNS_5SagTTILj64ELb1EEELNS_5OrderE0ELb0EE8merge_atEmmm
_ZN3nms6SorterIj2NxNS_5SagTTILj64ELb1EEELNS_5OrderE0ELb0EE8merge_atEmmm:
.LFB1613:
	pushq	%r15
	.seh_pushreg	%r15
	pushq	%r14
	.seh_pushreg	%r14
	pushq	%r13
	.seh_pushreg	%r13
	pushq	%r12
	.seh_pushreg	%r12
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$136, %rsp
	.seh_stackalloc	136
	.seh_endprologue
	movq	(%rcx), %rax
	movq	%r8, 56(%rsp)
	leaq	(%rax,%rdx,4), %rax
	movq	%r8, %rdi
	movq	%rcx, %rbx
	leaq	0(,%r8,4), %r8
	movq	%rax, 80(%rsp)
	addq	%r8, %rax
	movq	%r9, 48(%rsp)
	movq	%rax, 72(%rsp)
	movl	-4(%rax), %esi
	cmpl	%esi, (%rax)
	jnb	.L533
	movq	112(%rcx), %rcx
	movq	120(%rbx), %rax
	leaq	(%rdi,%r9), %rsi
	subq	%rcx, %rax
	sarq	$2, %rax
	cmpq	%rdi, %r9
	jb	.L535
	cmpq	%rdi, %rax
	jb	.L725
.L536:
	movq	80(%rsp), %rdx
	call	memcpy
	movq	112(%rbx), %rax
	movq	$0, 120(%rsp)
	movq	%rax, 88(%rsp)
	cmpq	$1, %rsi
	jbe	.L538
	leaq	-1(%rsi), %rax
	bsrq	%rax, %rax
	addl	$1, %eax
	cltq
	imulq	%rax, %rax
	movq	%rax, 120(%rsp)
.L538:
	movq	88(%rsp), %rax
	movq	72(%rsp), %r14
	movq	80(%rsp), %r11
	movl	(%rax), %esi
	cmpl	%esi, (%r14)
	jb	.L726
	movq	56(%rsp), %rax
	movq	72(%rsp), %rdx
	xorl	%esi, %esi
	xorl	%r13d, %r13d
	movq	88(%rsp), %r14
	movq	120(%rsp), %rbp
	xorl	%r12d, %r12d
	movq	$0, 64(%rsp)
	movq	%rax, 96(%rsp)
	xorl	%eax, %eax
	jmp	.L539
.L735:
	movq	112(%rsp), %rcx
	cmpq	$8, %rcx
	ja	.L574
	testq	%rcx, %rcx
	je	.L533
	movq	96(%rsp), %r8
	xorl	%eax, %eax
	.p2align 5
	.p2align 4
	.p2align 3
.L575:
	movl	(%r8,%rax,4), %edx
	movl	%edx, (%r11,%rax,4)
	addq	$1, %rax
	cmpq	%rcx, %rax
	jne	.L575
.L533:
	addq	$136, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
	.p2align 4,,10
	.p2align 3
.L535:
	movq	48(%rsp), %rdi
	cmpq	%rdi, %rax
	jb	.L727
.L578:
	movq	48(%rsp), %rdi
	movq	72(%rsp), %rdx
	leaq	0(,%rdi,4), %r8
	call	memcpy
	movq	80(%rsp), %rcx
	leaq	-1(%rsi), %rax
	movq	$0, 120(%rsp)
	leaq	(%rcx,%rax,4), %rdx
	movq	%rdx, 88(%rsp)
	movq	56(%rsp), %rdx
	leaq	-4(%rcx,%rdx,4), %rcx
	movq	112(%rbx), %rdx
	movq	%rcx, 72(%rsp)
	leaq	-4(%rdx,%rdi,4), %rdi
	movq	%rdi, 80(%rsp)
	cmpq	$1, %rsi
	jbe	.L580
	bsrq	%rax, %rax
	addl	$1, %eax
	cltq
	imulq	%rax, %rax
	movq	%rax, 120(%rsp)
.L580:
	movq	72(%rsp), %r11
	movq	80(%rsp), %rax
	movl	(%r11), %esi
	cmpl	%esi, (%rax)
	jnb	.L633
	movq	%rax, 96(%rsp)
	movq	48(%rsp), %rax
	xorl	%esi, %esi
	xorl	%r13d, %r13d
	movq	88(%rsp), %r9
	movq	120(%rsp), %rbp
	xorl	%r12d, %r12d
	movq	$0, 64(%rsp)
	movq	%rax, 112(%rsp)
	xorl	%eax, %eax
	.p2align 4
	.p2align 3
.L582:
	movq	96(%rsp), %rdi
	movq	%rbp, %r14
	movl	(%rdi), %edx
	movq	56(%rsp), %rdi
	subq	64(%rsp), %rdi
	leaq	-1(%rdi), %rcx
	movq	%rdi, 104(%rsp)
	movq	72(%rsp), %rdi
	cmpq	%rbp, %rcx
	cmovbe	%rcx, %r14
	leaq	-4(%rdi,%rax), %r10
	testq	%r14, %r14
	je	.L599
	movq	%r10, %rax
	xorl	%ebx, %ebx
	leaq	1(%rsi), %r15
	jmp	.L601
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L729:
	subq	$4, %rax
	cmpq	%rbx, %r14
	je	.L728
.L601:
	leaq	(%r15,%rbx), %rdi
	movq	%rbx, %r8
	addq	$1, %rbx
	cmpl	(%rax), %edx
	jb	.L729
	cmpq	$8, %rbx
	ja	.L603
.L622:
	addq	%rbx, 64(%rsp)
.L604:
	movq	%rbx, %rdx
	xorl	%eax, %eax
	negq	%rdx
	salq	$2, %rdx
	.p2align 5
	.p2align 4
	.p2align 3
.L612:
	movl	(%r11,%rax), %ecx
	movl	%ecx, (%r9,%rax)
	subq	$4, %rax
	cmpq	%rdx, %rax
	jne	.L612
.L722:
	addq	%rbx, %r13
	movq	88(%rsp), %rcx
	movq	%r13, %rax
	negq	%rax
	leaq	(%rcx,%rax,4), %r9
	movq	64(%rsp), %rcx
	cmpq	%rcx, 56(%rsp)
	je	.L730
.L613:
	movq	%rdi, %r9
	subq	%rsi, %r9
	cmpq	$63, %rbx
	ja	.L649
	cmpq	%r9, %rbx
	jb	.L649
.L719:
	movq	48(%rsp), %rax
	movq	80(%rsp), %rsi
	movq	%r13, %rcx
	movq	64(%rsp), %rdx
	negq	%rcx
	subq	%r12, %rax
	movq	%rax, 96(%rsp)
	movq	%r12, %rax
	negq	%rdx
	negq	%rax
	salq	$2, %rax
	leaq	(%rsi,%rax), %r10
	movq	72(%rsp), %rsi
	leaq	(%rsi,%rdx,4), %rdx
	movq	88(%rsp), %rsi
	leaq	(%rsi,%rcx,4), %r11
.L581:
	movq	96(%rsp), %rsi
	movq	%rbp, %r14
	movl	(%rdx), %edx
	leaq	-1(%rsi), %rcx
	movq	80(%rsp), %rsi
	cmpq	%rbp, %rcx
	cmovbe	%rcx, %r14
	leaq	-4(%rsi,%rax), %r9
	testq	%r14, %r14
	je	.L583
	movq	%r9, %rax
	xorl	%ebx, %ebx
	leaq	1(%rdi), %r15
	jmp	.L585
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L732:
	subq	$4, %rax
	cmpq	%rbx, %r14
	je	.L731
.L585:
	leaq	(%r15,%rbx), %rsi
	movq	%rbx, %r8
	addq	$1, %rbx
	cmpl	%edx, (%rax)
	jnb	.L732
	cmpq	$8, %rbx
	ja	.L587
.L623:
	addq	%rbx, %r12
.L588:
	movq	%rbx, %rdx
	xorl	%eax, %eax
	negq	%rdx
	salq	$2, %rdx
	.p2align 5
	.p2align 4
	.p2align 3
.L596:
	movl	(%r10,%rax), %ecx
	movl	%ecx, (%r11,%rax)
	subq	$4, %rax
	cmpq	%rdx, %rax
	jne	.L596
.L720:
	addq	%rbx, %r13
.L595:
	cmpq	%r12, 48(%rsp)
	je	.L533
	movq	%rsi, %r9
	subq	%rdi, %r9
	cmpq	$63, %rbx
	ja	.L648
	cmpq	%r9, %rbx
	jb	.L648
.L721:
	movq	48(%rsp), %rax
	movq	80(%rsp), %rdi
	movq	%r13, %rdx
	negq	%rdx
	subq	%r12, %rax
	movq	%rax, 112(%rsp)
	movq	%r12, %rax
	negq	%rax
	leaq	(%rdi,%rax,4), %rax
	movq	72(%rsp), %rdi
	movq	%rax, 96(%rsp)
	movq	64(%rsp), %rax
	negq	%rax
	salq	$2, %rax
	leaq	(%rdi,%rax), %r11
	movq	88(%rsp), %rdi
	leaq	(%rdi,%rdx,4), %r9
	jmp	.L582
.L633:
	movq	48(%rsp), %rax
	movq	88(%rsp), %r11
	xorl	%edi, %edi
	xorl	%r13d, %r13d
	movq	72(%rsp), %rdx
	movq	80(%rsp), %r10
	xorl	%r12d, %r12d
	movq	$0, 64(%rsp)
	movq	%rax, 96(%rsp)
	movq	120(%rsp), %rbp
	xorl	%eax, %eax
	jmp	.L581
.L726:
	movq	%rax, 96(%rsp)
	movq	56(%rsp), %rax
	xorl	%edi, %edi
	xorl	%r13d, %r13d
	movq	$0, 64(%rsp)
	movq	120(%rsp), %rbp
	xorl	%r12d, %r12d
	movq	%rax, 112(%rsp)
	xorl	%eax, %eax
	.p2align 4
	.p2align 3
.L540:
	movq	96(%rsp), %rsi
	movl	(%rsi), %edx
	movq	48(%rsp), %rsi
	subq	64(%rsp), %rsi
	leaq	-1(%rsi), %r8
	movq	%rsi, 104(%rsp)
	movq	72(%rsp), %rsi
	cmpq	%r8, %rbp
	movq	%r8, %r9
	cmovbe	%rbp, %r9
	leaq	4(%rsi,%rax), %rcx
	testq	%r9, %r9
	je	.L559
	xorl	%ebx, %ebx
	leaq	1(%rdi), %r10
	jmp	.L561
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L734:
	cmpq	%rbx, %r9
	je	.L733
.L561:
	movl	(%rcx,%rbx,4), %eax
	leaq	(%r10,%rbx), %rsi
	movq	%rbx, %r15
	addq	$1, %rbx
	cmpl	%edx, %eax
	jb	.L734
	cmpq	$8, %rbx
	ja	.L563
.L620:
	addq	%rbx, 64(%rsp)
	xorl	%eax, %eax
	.p2align 5
	.p2align 4
	.p2align 3
.L572:
	movl	(%r14,%rax,4), %edx
	movl	%edx, (%r11,%rax,4)
	addq	$1, %rax
	cmpq	%rbx, %rax
	jne	.L572
.L718:
	movq	80(%rsp), %rax
	addq	%rbx, %r13
	leaq	(%rax,%r13,4), %r11
.L571:
	movq	64(%rsp), %rcx
	cmpq	%rcx, 48(%rsp)
	je	.L735
	movq	%rsi, %r9
	subq	%rdi, %r9
	cmpq	$63, %rbx
	ja	.L647
	cmpq	%r9, %rbx
	jb	.L647
.L715:
	movq	56(%rsp), %rax
	movq	88(%rsp), %rdi
	movq	64(%rsp), %rcx
	subq	%r12, %rax
	movq	%rax, 96(%rsp)
	leaq	0(,%r12,4), %rax
	leaq	(%rdi,%rax), %r14
	movq	72(%rsp), %rdi
	leaq	(%rdi,%rcx,4), %rdx
	movq	80(%rsp), %rdi
	leaq	(%rdi,%r13,4), %r11
.L539:
	movq	96(%rsp), %rdi
	movq	%rbp, %r9
	movl	(%rdx), %edx
	leaq	-1(%rdi), %r8
	movq	88(%rsp), %rdi
	cmpq	%rbp, %r8
	cmovbe	%r8, %r9
	leaq	4(%rdi,%rax), %rcx
	testq	%r9, %r9
	je	.L541
	xorl	%ebx, %ebx
	leaq	1(%rsi), %r10
	jmp	.L543
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L737:
	cmpq	%rbx, %r9
	je	.L736
.L543:
	movl	(%rcx,%rbx,4), %eax
	leaq	(%r10,%rbx), %rdi
	movq	%rbx, %r15
	addq	$1, %rbx
	cmpl	%eax, %edx
	jnb	.L737
	cmpq	$8, %rbx
	ja	.L547
.L621:
	addq	%rbx, %r12
	xorl	%eax, %eax
	.p2align 5
	.p2align 4
	.p2align 3
.L556:
	movl	(%r14,%rax,4), %edx
	movl	%edx, (%r11,%rax,4)
	addq	$1, %rax
	cmpq	%rbx, %rax
	jne	.L556
.L716:
	addq	%rbx, %r13
.L555:
	cmpq	%r12, 56(%rsp)
	je	.L533
	movq	%rdi, %r9
	subq	%rsi, %r9
	cmpq	$63, %rbx
	ja	.L646
	cmpq	%r9, %rbx
	jb	.L646
.L717:
	movq	56(%rsp), %rax
	movq	72(%rsp), %rsi
	subq	%r12, %rax
	movq	%rax, 112(%rsp)
	movq	88(%rsp), %rax
	leaq	(%rax,%r12,4), %rax
	movq	%rax, 96(%rsp)
	movq	64(%rsp), %rax
	salq	$2, %rax
	leaq	(%rsi,%rax), %r14
	movq	80(%rsp), %rsi
	leaq	(%rsi,%r13,4), %r11
	jmp	.L540
	.p2align 4,,10
	.p2align 3
.L731:
	cmpq	%rbx, %rcx
	je	.L738
.L586:
	leaq	1(%rbp), %rax
	movl	$1, %r8d
	cmpq	%rax, %rcx
	movq	%rax, %r14
	cmovbe	%rcx, %r14
	addq	$1, %rsi
	movq	%r14, %rbx
	negq	%rbx
	cmpl	%edx, 4(%r9,%rbx,4)
	jnb	.L589
	jmp	.L739
	.p2align 6
	.p2align 4,,10
	.p2align 3
.L740:
	addq	%r8, %r8
	leaq	(%r8,%rbp), %rax
	cmpq	%rax, %rcx
	movq	%rax, %rbx
	cmovbe	%rcx, %rbx
	addq	$1, %rsi
	movq	%rbx, %r15
	negq	%r15
	cmpl	%edx, 4(%r9,%r15,4)
	jb	.L593
	movq	%rbx, %r14
.L589:
	cmpq	%rcx, %rax
	jb	.L740
	movq	96(%rsp), %rbx
.L592:
	cmpq	$8, %rbx
	ja	.L638
	testq	%rbx, %rbx
	jne	.L623
	jmp	.L595
	.p2align 4,,10
	.p2align 3
.L733:
	cmpq	%rbx, %r8
	je	.L741
.L562:
	leaq	1(%rbp), %rax
	movq	%r8, %r10
	movl	$1, %r9d
	cmpq	%r8, %rax
	cmovbe	%rax, %r10
	addq	$1, %rsi
	cmpl	%edx, -4(%rcx,%r10,4)
	jb	.L565
	jmp	.L742
	.p2align 6
	.p2align 4,,10
	.p2align 3
.L743:
	addq	%r9, %r9
	leaq	0(%rbp,%r9), %rax
	cmpq	%rax, %r8
	movq	%rax, %rbx
	cmovbe	%r8, %rbx
	addq	$1, %rsi
	cmpl	%edx, -4(%rcx,%rbx,4)
	jnb	.L569
	movq	%rbx, %r10
.L565:
	cmpq	%r8, %rax
	jb	.L743
	movq	104(%rsp), %rbx
.L568:
	cmpq	$8, %rbx
	ja	.L563
	testq	%rbx, %rbx
	jne	.L620
	jmp	.L571
	.p2align 4,,10
	.p2align 3
.L728:
	cmpq	%rbx, %rcx
	je	.L744
.L602:
	leaq	1(%rbp), %rax
	movl	$1, %r8d
	cmpq	%rax, %rcx
	movq	%rax, %r14
	cmovbe	%rcx, %r14
	addq	$1, %rdi
	movq	%r14, %rbx
	negq	%rbx
	cmpl	4(%r10,%rbx,4), %edx
	jb	.L605
	jmp	.L745
	.p2align 6
	.p2align 4,,10
	.p2align 3
.L746:
	addq	%r8, %r8
	leaq	(%r8,%rbp), %rax
	cmpq	%rax, %rcx
	movq	%rax, %rbx
	cmovbe	%rcx, %rbx
	addq	$1, %rdi
	movq	%rbx, %r15
	negq	%r15
	cmpl	4(%r10,%r15,4), %edx
	jnb	.L609
	movq	%rbx, %r14
.L605:
	cmpq	%rcx, %rax
	jb	.L746
	movq	104(%rsp), %rbx
.L608:
	cmpq	$8, %rbx
	ja	.L643
	testq	%rbx, %rbx
	jne	.L622
	movq	64(%rsp), %rcx
	cmpq	%rcx, 56(%rsp)
	jne	.L613
.L730:
	movq	112(%rsp), %rax
	cmpq	$8, %rax
	ja	.L614
	testq	%rax, %rax
	je	.L533
	movq	%rax, %rcx
	movq	96(%rsp), %r8
	xorl	%eax, %eax
	negq	%rcx
	salq	$2, %rcx
	.p2align 5
	.p2align 4
	.p2align 3
.L615:
	movl	(%r8,%rax), %edx
	movl	%edx, (%r9,%rax)
	subq	$4, %rax
	cmpq	%rcx, %rax
	jne	.L615
	jmp	.L533
	.p2align 4,,10
	.p2align 3
.L736:
	cmpq	%rbx, %r8
	je	.L747
.L546:
	leaq	1(%rbp), %rax
	movl	$1, %r9d
	cmpq	%rax, %r8
	movq	%rax, %r10
	cmovbe	%r8, %r10
	addq	$1, %rdi
	cmpl	-4(%rcx,%r10,4), %edx
	jnb	.L549
	jmp	.L748
	.p2align 6
	.p2align 4,,10
	.p2align 3
.L749:
	addq	%r9, %r9
	leaq	(%r9,%rbp), %rax
	cmpq	%rax, %r8
	movq	%rax, %rbx
	cmovbe	%r8, %rbx
	addq	$1, %rdi
	cmpl	-4(%rcx,%rbx,4), %edx
	jb	.L553
	movq	%rbx, %r10
.L549:
	cmpq	%r8, %rax
	jb	.L749
	movq	96(%rsp), %rbx
.L552:
	cmpq	$8, %rbx
	ja	.L547
	testq	%rbx, %rbx
	jne	.L621
	jmp	.L555
	.p2align 4,,10
	.p2align 3
.L643:
	movq	%rcx, %r8
.L603:
	negq	%r8
	salq	$2, %r8
	leaq	(%r11,%r8), %rdx
	leaq	(%r9,%r8), %rcx
	leaq	0(,%rbx,4), %r8
	call	memmove
	addq	%rbx, 64(%rsp)
	jmp	.L722
	.p2align 4,,10
	.p2align 3
.L638:
	movq	%rcx, %r8
.L587:
	negq	%r8
	addq	%rbx, %r12
	salq	$2, %r8
	leaq	(%r10,%r8), %rdx
	leaq	(%r11,%r8), %rcx
	leaq	0(,%rbx,4), %r8
	call	memmove
	jmp	.L720
	.p2align 4,,10
	.p2align 3
.L747:
	leaq	2(%r15), %rbx
	cmpq	$8, %rbx
	jbe	.L621
	.p2align 4
	.p2align 3
.L547:
	leaq	0(,%rbx,4), %r8
	movq	%r14, %rdx
	movq	%r11, %rcx
	addq	%rbx, %r12
	call	memmove
	jmp	.L716
	.p2align 4,,10
	.p2align 3
.L741:
	leaq	2(%r15), %rbx
	cmpq	$8, %rbx
	jbe	.L620
	.p2align 4
	.p2align 3
.L563:
	leaq	0(,%rbx,4), %r8
	movq	%r14, %rdx
	movq	%r11, %rcx
	call	memmove
	addq	%rbx, 64(%rsp)
	jmp	.L718
.L742:
	movq	%r10, %rbx
	movq	%rbp, %r10
	.p2align 4
	.p2align 3
.L569:
	movq	%rbx, %rax
	subq	%r10, %rax
	cmpq	$1, %rax
	ja	.L566
	jmp	.L568
	.p2align 6
	.p2align 4,,10
	.p2align 3
.L750:
	movq	%rbx, %rax
	movq	%r8, %r10
	subq	%r8, %rax
.L570:
	cmpq	$1, %rax
	jbe	.L568
.L566:
	shrq	%rax
	addq	$1, %rsi
	leaq	(%rax,%r10), %r8
	cmpl	%edx, -4(%rcx,%r8,4)
	jb	.L750
	movq	%r8, %rbx
	jmp	.L570
.L739:
	movq	%r14, %rbx
	movq	%rbp, %r14
	.p2align 4
	.p2align 3
.L593:
	movq	%rbx, %rax
	subq	%r14, %rax
	cmpq	$1, %rax
	ja	.L590
	jmp	.L591
	.p2align 6
	.p2align 4,,10
	.p2align 3
.L751:
	movq	%rbx, %rax
	movq	%rcx, %r14
	subq	%rcx, %rax
	cmpq	$1, %rax
	jbe	.L591
.L590:
	shrq	%rax
	addq	$1, %rsi
	leaq	(%rax,%r14), %rcx
	movq	%rcx, %r8
	negq	%r8
	cmpl	%edx, 4(%r9,%r8,4)
	jnb	.L751
	movq	%rcx, %rbx
	cmpq	$1, %rax
	ja	.L590
.L591:
	leaq	-1(%rbx), %rcx
	jmp	.L592
.L745:
	movq	%r14, %rbx
	movq	%rbp, %r14
	.p2align 4
	.p2align 3
.L609:
	movq	%rbx, %rax
	subq	%r14, %rax
	cmpq	$1, %rax
	ja	.L606
	jmp	.L607
	.p2align 6
	.p2align 4,,10
	.p2align 3
.L752:
	movq	%rbx, %rax
	movq	%rcx, %r14
	subq	%rcx, %rax
	cmpq	$1, %rax
	jbe	.L607
.L606:
	shrq	%rax
	addq	$1, %rdi
	leaq	(%rax,%r14), %rcx
	movq	%rcx, %r8
	negq	%r8
	cmpl	4(%r10,%r8,4), %edx
	jb	.L752
	movq	%rcx, %rbx
	cmpq	$1, %rax
	ja	.L606
.L607:
	leaq	-1(%rbx), %rcx
	jmp	.L608
.L748:
	movq	%r10, %rbx
	movq	%rbp, %r10
	.p2align 4
	.p2align 3
.L553:
	movq	%rbx, %rax
	subq	%r10, %rax
	cmpq	$1, %rax
	ja	.L550
	jmp	.L552
	.p2align 6
	.p2align 4,,10
	.p2align 3
.L753:
	movq	%rbx, %rax
	movq	%r8, %r10
	subq	%r8, %rax
.L554:
	cmpq	$1, %rax
	jbe	.L552
.L550:
	shrq	%rax
	addq	$1, %rdi
	leaq	(%rax,%r10), %r8
	cmpl	-4(%rcx,%r8,4), %edx
	jnb	.L753
	movq	%r8, %rbx
	jmp	.L554
	.p2align 4,,10
	.p2align 3
.L744:
	leaq	2(%r8), %rax
	cmpq	$8, %rax
	ja	.L639
	addq	%rax, 64(%rsp)
	movq	%rax, %rbx
	jmp	.L604
	.p2align 4,,10
	.p2align 3
.L738:
	leaq	2(%r8), %rax
	cmpq	$8, %rax
	ja	.L634
	addq	%rax, %r12
	movq	%rax, %rbx
	jmp	.L588
	.p2align 4,,10
	.p2align 3
.L647:
	movq	104(%rsp), %rax
	movq	120(%rsp), %rdx
	movq	%rbp, %rcx
	movq	%rbx, %r8
	movq	%rax, 32(%rsp)
	call	_ZN3nms5SagTTILj64ELb1EE5adaptEmmmmm
	movq	%rax, %rbp
	jmp	.L715
	.p2align 4,,10
	.p2align 3
.L648:
	movq	96(%rsp), %rax
	movq	120(%rsp), %rdx
	movq	%rbp, %rcx
	movq	%rbx, %r8
	movq	%rax, 32(%rsp)
	call	_ZN3nms5SagTTILj64ELb1EE5adaptEmmmmm
	movq	%rax, %rbp
	jmp	.L721
	.p2align 4,,10
	.p2align 3
.L646:
	movq	96(%rsp), %rax
	movq	120(%rsp), %rdx
	movq	%rbp, %rcx
	movq	%rbx, %r8
	movq	%rax, 32(%rsp)
	call	_ZN3nms5SagTTILj64ELb1EE5adaptEmmmmm
	movq	%rax, %rbp
	jmp	.L717
	.p2align 4,,10
	.p2align 3
.L649:
	movq	104(%rsp), %rax
	movq	120(%rsp), %rdx
	movq	%rbp, %rcx
	movq	%rbx, %r8
	movq	%rax, 32(%rsp)
	call	_ZN3nms5SagTTILj64ELb1EE5adaptEmmmmm
	movq	%rax, %rbp
	jmp	.L719
.L599:
	movq	%rsi, %rdi
	testq	%rcx, %rcx
	jne	.L602
	addq	$1, 64(%rsp)
	movl	$1, %ebx
	jmp	.L604
.L583:
	movq	%rdi, %rsi
	testq	%rcx, %rcx
	jne	.L586
	addq	$1, %r12
	movl	$1, %ebx
	jmp	.L588
.L541:
	movq	%rsi, %rdi
	testq	%r8, %r8
	jne	.L546
	movl	(%r14), %eax
	addq	$1, %r12
	movl	%eax, (%r11)
	cmpq	%r12, 56(%rsp)
	je	.L533
	addq	$1, %r13
	jmp	.L717
.L559:
	movq	%rdi, %rsi
	testq	%r8, %r8
	jne	.L562
	movl	(%r14), %eax
	addq	$1, 64(%rsp)
	movl	$1, %ebx
	movl	%eax, (%r11)
	jmp	.L718
.L725:
	movq	%rdi, %rdx
	leaq	112(%rbx), %rcx
	movq	%r8, 64(%rsp)
	subq	%rax, %rdx
	call	_ZNSt6vectorIjSaIjEE17_M_default_appendEm
	movq	32(%rbx), %rax
	movq	112(%rbx), %rcx
	movq	64(%rsp), %r8
	cmpq	%rdi, 48(%rax)
	jnb	.L536
	movq	%rdi, 48(%rax)
	jmp	.L536
.L727:
	movq	%rdi, %rdx
	leaq	112(%rbx), %rcx
	subq	%rax, %rdx
	call	_ZNSt6vectorIjSaIjEE17_M_default_appendEm
	movq	32(%rbx), %rax
	movq	112(%rbx), %rcx
	cmpq	%rdi, 48(%rax)
	jnb	.L578
	movq	%rdi, 48(%rax)
	jmp	.L578
.L639:
	movq	%rbx, %r8
	movq	%rax, %rbx
	jmp	.L603
.L634:
	movq	%rbx, %r8
	movq	%rax, %rbx
	jmp	.L587
.L614:
	movq	96(%rsp), %rdx
	subq	48(%rsp), %r12
	leaq	4(,%r12,4), %rax
	addq	%rax, %rdx
	leaq	(%r9,%rax), %rcx
	movq	112(%rsp), %rax
	leaq	0(,%rax,4), %r8
.L723:
	addq	$136, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	jmp	memmove
.L574:
	movq	112(%rsp), %r8
	movq	96(%rsp), %rdx
	movq	%r11, %rcx
	salq	$2, %r8
	jmp	.L723
	.seh_endproc
	.section	.text$_ZN3nms6SorterIj2NxNS_5SagTTILj64ELb1EEELNS_5OrderE0ELb0EE9run_powerEv,"x"
	.linkonce discard
	.align 2
	.p2align 4
	.globl	_ZN3nms6SorterIj2NxNS_5SagTTILj64ELb1EEELNS_5OrderE0ELb0EE9run_powerEv
	.def	_ZN3nms6SorterIj2NxNS_5SagTTILj64ELb1EEELNS_5OrderE0ELb0EE9run_powerEv;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN3nms6SorterIj2NxNS_5SagTTILj64ELb1EEELNS_5OrderE0ELb0EE9run_powerEv
_ZN3nms6SorterIj2NxNS_5SagTTILj64ELb1EEELNS_5OrderE0ELb0EE9run_powerEv:
.LFB1584:
	pushq	%r15
	.seh_pushreg	%r15
	pushq	%r14
	.seh_pushreg	%r14
	pushq	%r13
	.seh_pushreg	%r13
	pushq	%r12
	.seh_pushreg	%r12
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$2664, %rsp
	.seh_stackalloc	2664
	.seh_endprologue
	xorl	%r8d, %r8d
	leaq	80(%rsp), %rax
	movq	%rcx, %rdx
	movq	%rcx, %rdi
	movq	%rax, %rcx
	movq	%rax, 64(%rsp)
	call	_ZN3nms6SorterIj2NxNS_5SagTTILj64ELb1EEELNS_5OrderE0ELb0EE8next_runEm
	movq	88(%rsp), %r9
	cmpq	8(%rdi), %r9
	jnb	.L754
	movq	%r9, 32(%rsp)
	leaq	96(%rsp), %rax
	movq	80(%rsp), %r14
	xorl	%r13d, %r13d
	movq	%rax, 72(%rsp)
	leaq	608(%rsp), %rsi
	movq	%r9, %rax
	.p2align 4
	.p2align 3
.L763:
	movq	32(%rsp), %rbx
	movq	64(%rsp), %rcx
	movq	%rdi, %rdx
	xorl	%ebp, %ebp
	movq	%rax, 48(%rsp)
	movq	%rbx, %r8
	call	_ZN3nms6SorterIj2NxNS_5SagTTILj64ELb1EEELNS_5OrderE0ELb0EE8next_runEm
	movq	80(%rsp), %rax
	movq	48(%rsp), %r12
	movq	8(%rdi), %r10
	movq	%rax, 40(%rsp)
	movq	88(%rsp), %rax
	leaq	(%r12,%r14,2), %rdx
	addq	%rax, %rbx
	leaq	(%rax,%r12), %r8
	movq	%rbx, 32(%rsp)
	addq	%rdx, %r8
	jmp	.L760
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L768:
	subq	%r10, %rdx
	subq	%r10, %r8
.L757:
	addq	%rdx, %rdx
	addq	%r8, %r8
.L760:
	addl	$1, %ebp
	cmpq	%r10, %rdx
	jnb	.L768
	cmpq	%r10, %r8
	jb	.L757
	testl	%r13d, %r13d
	jle	.L769
	movq	72(%rsp), %rcx
	leal	-1(%r13), %ebx
	movq	%rsi, 56(%rsp)
	movq	%rax, 48(%rsp)
	leaq	(%rcx,%rbx,4), %r15
	salq	$4, %rbx
	addq	%rsi, %rbx
	movl	%r13d, %esi
	jmp	.L761
	.p2align 4,,10
	.p2align 3
.L762:
	movq	8(%rbx), %r13
	movq	(%rbx), %r14
	movq	%r12, %r9
	movq	%rdi, %rcx
	subl	$1, %esi
	subq	$4, %r15
	subq	$16, %rbx
	movq	%r13, %r8
	movq	%r14, %rdx
	addq	%r13, %r12
	call	_ZN3nms6SorterIj2NxNS_5SagTTILj64ELb1EEELNS_5OrderE0ELb0EE8merge_atEmmm
	testl	%esi, %esi
	je	.L770
.L761:
	cmpl	%ebp, (%r15)
	jnb	.L762
	movslq	%esi, %rdx
	leal	1(%rsi), %r13d
	movq	48(%rsp), %rax
	movq	8(%rdi), %r10
	movq	56(%rsp), %rsi
.L759:
	movq	%rdx, %r8
	movl	%ebp, 96(%rsp,%rdx,4)
	salq	$4, %r8
	movq	%r14, 608(%rsp,%r8)
	movq	40(%rsp), %r14
	movq	%r12, 616(%rsp,%r8)
	cmpq	%r10, 32(%rsp)
	jb	.L763
	movq	%rax, %r9
	testl	%r13d, %r13d
	jle	.L754
	leal	-1(%r13), %eax
	leaq	592(%rsp), %rbp
	movq	%r9, %rsi
	salq	$4, %rax
	leaq	608(%rsp,%rax), %rbx
	.p2align 4
	.p2align 3
.L764:
	movq	8(%rbx), %r12
	movq	(%rbx), %rdx
	movq	%rsi, %r9
	movq	%rdi, %rcx
	subq	$16, %rbx
	movq	%r12, %r8
	addq	%r12, %rsi
	call	_ZN3nms6SorterIj2NxNS_5SagTTILj64ELb1EEELNS_5OrderE0ELb0EE8merge_atEmmm
	cmpq	%rbp, %rbx
	jne	.L764
.L754:
	addq	$2664, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
	.p2align 4,,10
	.p2align 3
.L770:
	movq	48(%rsp), %rax
	movq	56(%rsp), %rsi
	xorl	%edx, %edx
	movl	$1, %r13d
	movq	8(%rdi), %r10
	jmp	.L759
.L769:
	movslq	%r13d, %rdx
	addl	$1, %r13d
	jmp	.L759
	.seh_endproc
	.text
	.p2align 4
	.globl	_Z9probe_sagPjm
	.def	_Z9probe_sagPjm;	.scl	2;	.type	32;	.endef
	.seh_proc	_Z9probe_sagPjm
_Z9probe_sagPjm:
.LFB1527:
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$176, %rsp
	.seh_stackalloc	176
	.seh_endprologue
	pxor	%xmm0, %xmm0
	leaq	72(%rsp), %rax
	movq	%rcx, 32(%rsp)
	leaq	32(%rsp), %rcx
	movq	%rdx, 40(%rsp)
	movq	$0, 48(%rsp)
	movq	$0, 56(%rsp)
	movq	$0, 120(%rsp)
	movq	$0, 128(%rsp)
	movq	$32, 136(%rsp)
	movq	$0, 160(%rsp)
	movq	%rax, 64(%rsp)
	movups	%xmm0, 72(%rsp)
	movups	%xmm0, 88(%rsp)
	movups	%xmm0, 104(%rsp)
	movaps	%xmm0, 144(%rsp)
	cmpq	$1, %rdx
	jbe	.L771
.LEHB8:
	call	_ZN3nms6SorterIj2NxNS_5SagTTILj64ELb1EEELNS_5OrderE0ELb0EE9run_powerEv
.LEHE8:
	movq	144(%rsp), %rcx
	testq	%rcx, %rcx
	je	.L771
	movq	160(%rsp), %rdx
	subq	%rcx, %rdx
	call	_ZdlPvm
	nop
.L771:
	addq	$176, %rsp
	popq	%rbx
	ret
.L776:
	movq	144(%rsp), %rcx
	movq	160(%rsp), %rdx
	movq	%rax, %rbx
	call	_ZNSt12_Vector_baseIjSaIjEED2Ev.isra.0
	movq	%rbx, %rcx
.LEHB9:
	call	_Unwind_Resume
	nop
.LEHE9:
	.seh_handler	__gxx_personality_seh0, @unwind, @except
	.seh_handlerdata
.LLSDA1527:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE1527-.LLSDACSB1527
.LLSDACSB1527:
	.uleb128 .LEHB8-.LFB1527
	.uleb128 .LEHE8-.LEHB8
	.uleb128 .L776-.LFB1527
	.uleb128 0
	.uleb128 .LEHB9-.LFB1527
	.uleb128 .LEHE9-.LEHB9
	.uleb128 0
	.uleb128 0
.LLSDACSE1527:
	.text
	.seh_endproc
	.def	__gxx_personality_seh0;	.scl	2;	.type	32;	.endef
	.ident	"GCC: (GNU) 15.3.0"
	.def	_ZdlPvm;	.scl	2;	.type	32;	.endef
	.def	memmove;	.scl	2;	.type	32;	.endef
	.def	memset;	.scl	2;	.type	32;	.endef
	.def	__real__Znwm;	.scl	2;	.type	32;	.endef
	.def	_Znwm;	.scl	2;	.type	32;	.endef
	.def	memcpy;	.scl	2;	.type	32;	.endef
	.def	_ZSt20__throw_length_errorPKc;	.scl	2;	.type	32;	.endef
	.def	_Unwind_Resume;	.scl	2;	.type	32;	.endef
	.def	memmove;	.scl	2;	.type	32;	.endef
