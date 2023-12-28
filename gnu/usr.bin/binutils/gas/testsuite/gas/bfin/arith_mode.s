	.text

	// Accumulator to Half D-register Moves

	R0.L = A0;
	R0.L = A0 (FU);
	R0.L = A0 (IS);
	R0.L = A0 (IU);
	R0.L = A0 (T);
	R0.L = A0 (TFU); // Not documented
	R0.L = A0 (S2RND);
	R0.L = A0 (ISS2);
	R0.L = A0 (IH);

	// Accumulator to D-register Moves

	R0 = A0;
	R0 = A0 (FU);
	R0 = A0 (IS); // Not documented
	R0 = A0 (IU); // Not documented
	R0 = A0 (S2RND);
	R0 = A0 (ISS2);

	// Multiply 16-Bit Operands to Half Dreg

	R0.H = R1.L * R2.H;
	R0.H = R1.L * R2.H (FU);
	R0.H = R1.L * R2.H (IS);
	R0.H = R1.L * R2.H (IU);
	R0.H = R1.L * R2.H (T);
	R0.H = R1.L * R2.H (TFU);
	R0.H = R1.L * R2.H (S2RND);
	R0.H = R1.L * R2.H (ISS2);
	R0.H = R1.L * R2.H (IH);

	// Multiply 16-Bit Operands to Dreg

	R0 = R1.L * R2.H;
	R0 = R1.L * R2.H (FU);
	R0 = R1.L * R2.H (IS);
	R0 = R1.L * R2.H (S2RND); // Not documented
	R0 = R1.L * R2.H (ISS2);

	// Multiply and Multiply-Accumulate to Accumulator

	A0 = R1.L * R2.H;
	A0 = R1.L * R2.H (FU);
	A0 = R1.L * R2.H (IS);
	A0 = R1.L * R2.H (W32);

	// Multiply and Multiply-Accumulate to Half-Register

	R0.L = (A0 = R1.L * R2.H);
	R0.L = (A0 = R1.L * R2.H) (FU);
	R0.L = (A0 = R1.L * R2.H) (IS);
	R0.L = (A0 = R1.L * R2.H) (IU);
	R0.L = (A0 = R1.L * R2.H) (T);
	R0.L = (A0 = R1.L * R2.H) (TFU);
	R0.L = (A0 = R1.L * R2.H) (S2RND);
	R0.L = (A0 = R1.L * R2.H) (ISS2);
	R0.L = (A0 = R1.L * R2.H) (IH);

	// Multiply and Multiply-Accumulate to Data Register

	R0 = (A0 = R1.L * R2.H);
	R0 = (A0 = R1.L * R2.H) (FU);
	R0 = (A0 = R1.L * R2.H) (IS);
	R0 = (A0 = R1.L * R2.H) (IU); // Not documented
	R0 = (A0 = R1.L * R2.H) (S2RND);
	R0 = (A0 = R1.L * R2.H) (ISS2);


