package hr.fer.bio.project.rankable;

import hr.fer.bio.project.wavelet.TreeNode;

/**
 * Created by Branimir on 12.1.2015..
 */
public interface TreeOperations {
    public int rank(char c, int endPos, TreeNode rootNode);
    public int select(char c, int boundary, TreeNode rootNode);
}