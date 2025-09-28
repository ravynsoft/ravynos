	.text

	// All available modes: FU, IS, IU, T, TFU, S2RND, ISS2, IH, W32

	// Accumulator to Half D-register Moves

	R0.L = A0 (W32);

	// Accumulator to D-register Moves

	R0 = A0 (T);
	R0 = A0 (TFU);
	R0 = A0 (IH);
	R0 = A0 (W32);

	// Multiply 16-Bit Operands to Half Dreg

	R0.H = R1.L * R2.H (W32);

	// Multiply 16-Bit Operands to Dreg

	R0 = R1.L * R2.H (IU);
	R0 = R1.L * R2.H (T);
	R0 = R1.L * R2.H (TFU);
	R0 = R1.L * R2.H (IH);
	R0 = R1.L * R2.H (W32);

	// Multiply and Multiply-Accumulate to Accumulator

	A0 = R1.L * R2.H (IU);
	A0 = R1.L * R2.H (T);
	A0 = R1.L * R2.H (TFU);
	A0 = R1.L * R2.H (S2RND);
	A0 = R1.L * R2.H (ISS2);
	A0 = R1.L * R2.H (IH);

	// Multiply and Multiply-Accumulate to Half-Register

	R0.L = (A0 = R1.L * R2.H) (W32);

	// Multiply and Multiply-Accumulate to Data Register

	R0 = (A0 = R1.L * R2.H) (T);
	R0 = (A0 = R1.L * R2.H) (TFU);
	R0 = (A0 = R1.L * R2.H) (IH);
	R0 = (A0 = R1.L * R2.H) (W32);
