#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <gmp.h>
#include "vrf.h"

int readfile(unsigned char *out, int len, const char *name) {
	int fd = 0;
	fd = open(name, O_RDONLY);
	if (fd == -1) {
		return 0;
	}
	ssize_t n = 0, dn = 0;
	do {
		dn = read(fd, out+n, len-n);
		if (dn <= 0) {
			break;
		}
		n += dn;
	} while(n < len);
	close(fd);
	return (n == len && dn >= 0);
}

int main(int argc, char **argv) {
	unsigned char pk[32], skpk[64], alpha[1], pi_good[80], beta[64];
	if (!readfile(pk, 32, "pk")) {
		fprintf(stderr, "pk read error\n");
		return (1);
	}
	// printf("%s", pk);

	if (!readfile(skpk, 32, "sk")) {
		fprintf(stderr, "sk read error\n");
		return (2);
	}
	memmove(skpk+32, pk, 32);
	if (!readfile(alpha, 1, "alpha")) {
		fprintf(stderr, "alpha read error\n");
		return (3);
	}
	if (!readfile(pi_good, 80, "pi")) {
		fprintf(stderr, "pi read error\n");
		return (4);
	}
	if (!readfile(beta, 64, "beta")) {
		fprintf(stderr, "beta read error\n");
		return (5);
	}

	// printf("alpha = %s", alpha);

	unsigned char pi_ours[80];
	int err = vrf_prove(pi_ours, skpk, alpha, sizeof alpha);
	if (err != 0) {
		fprintf(stderr, "prove() returned error\n");
		return (6);
	}
	// for(int i = 0; i < 80; i++){
	// 	printf("%c", pi_ours[i]);
	// }
	if (memcmp(pi_ours, pi_good, 80) != 0) {
		fprintf(stderr, "Produced wrong proof\n");
		return (7);
	}

	unsigned char hash[64];
	err = vrf_verify(hash, pk, pi_ours, alpha, sizeof alpha);
	if (err != 0) {
		fprintf(stderr, "Proof did not verify\n");
		return (8);
	}
	if (memcmp(hash, beta, 64) != 0) {
		fprintf(stderr, "verify() returned wrong hash\n");
		return (9);
	}

	fprintf(stderr, "PASS\n");
	fprintf(stderr, "%s\n", hash);

	unsigned char beta_ours[64];
	err = vrf_proof_to_hash(beta_ours, pi_ours);
	if (err != 0) {
		fprintf(stderr, "Proof_to_Hash() returned error\n");
		return (10);
	}
	if (memcmp(beta_ours, beta, 64) != 0) {
		fprintf(stderr, "Proof_to_Hash() returned wrong beta\n");
		return (11);
	}

	// Test Cryptographic sortition
	unsigned char output[64];
	unsigned char pi[80];
	double tau = 650;
	double W = 1100;
	
	err = cryptographic_sortition(output, pi, skpk, alpha, sizeof alpha, tau, W);
	if (err != 0) {
		fprintf(stderr, "Sortition() returned error\n");
		return (11);
	}

	unsigned char output_hex_str[129];
	bytesToHexString(output, output_hex_str, 64);
	printf("VRF output: %s\n", output);

	err = sortition_verify(pk, pi, alpha, sizeof alpha);
	if (err != 0) {
		fprintf(stderr, "Proof did not verify\n");
		return (12);
	}

	// // Define an mpf_t variable for the fraction result
    // mpf_t fraction;
    // mpf_init(fraction);

    // // Compute fraction
    // compute_fraction(hash, fraction);
	
    // gmp_printf("Fraction: %.50Ff\n", fraction);

	// double tau = 650;
	// double W = 1000;
	// double p = tau / W;

	// // Define an mpf_t variable for threshold and set it to `p`
    // mpf_t threshold;
    // mpf_init(threshold);
    // mpf_set_d(threshold, p);  // Convert double p to GMP floating-point

	// // Compare fraction with threshold
    // if (mpf_cmp(fraction, threshold) < 0) {
    //     printf("Fraction is less than %f\n", p);
    // } else {
    //     printf("Fraction is greater than or equal to %f\n", p);
    // }



	return 0;
}
