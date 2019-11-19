package us.ideasaur.jfuzzy;
/*
 * JNI bindings for the libfuzzy shared library.
 * Copyright (C) 2019 Russell Francis
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import java.io.File;
import java.io.IOException;
import java.io.InputStream;

/**
 * This class can be used to generate ssdeep hashes of buffers, files and streams using the underlying
 * native implementation provided by libfuzzy.so.
 */
public class JFuzzy {
  // Prevent construction, all methods are static.
  private JFuzzy() {}

  static private final int BUFFER_SIZE = 8192;

  // Define the native methods provided by libjfuzzy.so.
  static private native long fuzzy_new();
  static private native void fuzzy_free(long fuzzyState);
  static private native void fuzzy_update(long fuzzyState, byte[] buf, int length);
  static private native String fuzzy_digest(long fuzzyState, boolean FUZZY_FLAG_ELIMSEQ, boolean FUZZY_FLAG_NOTRUNC);
  static private native String fuzzy_hash_buf(byte[] buf);
  static private native String fuzzy_hash_file(String filename);
  static private native int fuzzy_compare(String sig1, String sig2);

  // Dynamically link the libjfuzzy.so JNI bindings when this class is loaded by the class loader.
  static {
    System.loadLibrary("jfuzzy");
  }

  /**
   * Given two ssdeep hashes, return a number between 0 and 100 indicating the similarity, where 0
   * indicates no similarity and 100 indicates a perfect match.
   * @param sig1 The first ssdeep signature to compare.
   * @param sig2 The second ssdeep signature to compare.
   * @return The similarity between the two signatures.
   * @throws RuntimeException If there is an error computing the similarity between the two hashes.
   */
  static public int compare(final String sig1, final String sig2) {
    return fuzzy_compare(sig1, sig2);
  }

  /**
   * Generate an ssdeep hash of the provided buffer.
   *
   * @param buf The byte[] which we wish to generate a hash of.
   * @return The ssdeep hash represented as a string for this data.
   * @throws RuntimeException If there is an error generating the hash.
   * @throws NullPointerException If the parameter buf is null.
   */
  static public String hash(final byte[] buf) {
    if (buf == null) {
      throw new NullPointerException("Parameter buf may not be null.");
    }
    return fuzzy_hash_buf(buf);
  }

  /**
   * Generate an ssdeep hash of the provided file.
   *
   * @param file The file to generate an ssdeep hash of.
   * @return The ssdeep hash of the provided file.
   * @throws RuntimeException If there is an error generating the hash.
   * @throws NullPointerException If the provided file is null.
   */
  static public String hash(final File file) {
    return fuzzy_hash_file(file.getAbsolutePath());
  }

  /**
   * Generate an ssdeep hash of the provided stream of data.
   *
   * @param inputStream The input stream from which to read data.
   * @return An ssdeep hash of the data within the provided stream.
   * @throws IOException If there is an error reading from the stream.
   * @throws NullPointerException If the inputStream is null.
   * @throws RuntimeException If there is another error generating the digest.
   */
  static public String hash(final InputStream inputStream) throws IOException {
    return hash(inputStream, false, false);
  }

  /**
   * Generate an ssdeep hash of the provided stream of data.
   *
   * @param inputStream The input stream from which to read data.
   * @param eliminateSequences true to eliminate sequences of more than 3 idential characters.
   * @param noTruncation true to indicate that you don't want the second portion of the hash truncated
   * to SPAMSUM_LENGTH/2 characters.
   * @return An ssdeep hash of the data within the provided stream.
   * @throws IOException If there is an error reading from the stream.
   * @throws NullPointerException If the inputStream is null.
   * @throws RuntimeException If there is another error generating the digest.
   */
  static public String hash(final InputStream inputStream, final boolean eliminateSequences, final boolean noTruncation)
      throws IOException {
    final long fuzzyState = fuzzy_new();
    try {
      final byte[] buf = new byte[BUFFER_SIZE];
      int bytesRead = inputStream.read(buf);
      while (bytesRead != -1) {
        fuzzy_update(fuzzyState, buf, bytesRead);
        bytesRead = inputStream.read(buf);
      }
      return fuzzy_digest(fuzzyState, eliminateSequences, noTruncation);
    } finally {
      fuzzy_free(fuzzyState);
    }
  }
}
