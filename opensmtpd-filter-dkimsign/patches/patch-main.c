$NetBSD: patch-main.c,v 1.1 2023/09/21 12:22:52 vins Exp $

Address signedness mismatch warnings.

--- main.c.orig	2022-01-27 21:56:05.000000000 +0000
+++ main.c
@@ -103,8 +103,8 @@ void dkim_adddomain(char *);
 void dkim_err(struct dkim_message *, char *);
 void dkim_errx(struct dkim_message *, char *);
 void dkim_headers_set(char *);
-void dkim_dataline(struct osmtpd_ctx *, const char *);
-void dkim_commit(struct osmtpd_ctx *);
+int dkim_dataline(struct osmtpd_ctx *, const char *);
+int dkim_commit(struct osmtpd_ctx *);
 void *dkim_message_new(struct osmtpd_ctx *);
 void dkim_message_free(struct osmtpd_ctx *, void *);
 void dkim_parse_header(struct dkim_message *, char *, int);
@@ -253,7 +253,7 @@ dkim_adddomain(char *d)
 	
 }
 
-void
+int
 dkim_dataline(struct osmtpd_ctx *ctx, const char *line)
 {
 	struct dkim_message *message = ctx->local_message;
@@ -263,7 +263,7 @@ dkim_dataline(struct osmtpd_ctx *ctx, co
 	if (message->err) {
 		if (line[0] == '.' && line[1] =='\0')
 			osmtpd_filter_dataline(ctx, ".");
-		return;
+		return 0;
 	}
 
 	linelen = strlen(line);
@@ -281,7 +281,7 @@ dkim_dataline(struct osmtpd_ctx *ctx, co
 		free(linedup);
 	} else if (linelen == 0 && message->parsing_headers) {
 		if (addheaders > 0 && !dkim_signature_printf(message, "; "))
-			return;
+			return 0;
 		message->parsing_headers = 0;
 	} else {
 		if (line[0] == '.')
@@ -291,9 +291,10 @@ dkim_dataline(struct osmtpd_ctx *ctx, co
 		dkim_parse_body(message, linedup);
 		free(linedup);
 	}
+	return 0;
 }
 
-void
+int
 dkim_commit(struct osmtpd_ctx *ctx)
 {
 	struct dkim_message *message = ctx->local_message;
@@ -302,6 +303,7 @@ dkim_commit(struct osmtpd_ctx *ctx)
 		osmtpd_filter_disconnect(ctx, "Internal server error");
 	else
 		osmtpd_filter_proceed(ctx);
+ 	return 0;
 }
 
 void *
@@ -594,6 +596,7 @@ dkim_sign(struct osmtpd_ctx *ctx)
 	ssize_t i;
 	size_t linelen = 0;
 	char *tmp, *tmp2;
+	unsigned char *utmp;
 	unsigned int digestsz;
 
 	if (addtime || addexpire)
@@ -717,18 +720,18 @@ dkim_sign(struct osmtpd_ctx *ctx)
 		}
 #endif
 	}
-	if ((tmp = malloc(linelen)) == NULL) {
+	if ((utmp = malloc(linelen)) == NULL) {
 		dkim_err(message, "Can't allocate space for signature");
 		goto fail;
 	}
 	if (!sephash) {
-		if (EVP_DigestSignFinal(message->dctx, tmp, &linelen) != 1) {
+		if (EVP_DigestSignFinal(message->dctx, utmp, &linelen) != 1) {
 			dkim_errx(message, "Failed to finalize signature");
 			goto fail;
 		}
 #ifdef HAVE_ED25519
 	} else {
-		if (EVP_DigestSign(message->dctx, tmp, &linelen, bdigest,
+		if (EVP_DigestSign(message->dctx, utmp, &linelen, bdigest,
 		    digestsz) != 1) {
 			dkim_errx(message, "Failed to finalize signature");
 			goto fail;
@@ -739,8 +742,8 @@ dkim_sign(struct osmtpd_ctx *ctx)
 		dkim_err(message, "Can't create DKIM signature");
 		goto fail;
 	}
-	EVP_EncodeBlock(b, tmp, linelen);
-	free(tmp);
+	EVP_EncodeBlock(b, utmp, linelen);
+	free(utmp);
 	dkim_signature_printf(message, "%s\r\n", b);
 	free(b);
 	dkim_signature_normalize(message);
