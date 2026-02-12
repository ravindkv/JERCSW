---

## **Histogram/TProfile/TGraph Naming Convention Summary**

### **Golden Rule**
- **Use `camelCase`:** Always employ `camelCase` for naming.
- **Avoid Double Capitals:** Do not have two capital letters side by side within names.

---

### **1D Objects**

**Naming Pattern:**
```
[type][1][VarOnY]In[VarOnX]
```

**Examples:**
- **Histogram of Events vs. Jet Pt:**
  ```
  h1EventInJetPt
  ```
- **TProfile of Response vs. Z Pt:**
  ```
  p1RespInZpt
  ```

**With Applied Cuts:**
- **Add `For[Var][CutValue]` to the Name**

**Examples:**
- **For Jet Eta < 1.33:**
  ```
  h1EventInJetPtForJetEta1p33
  p1RespInZptForJetEta1p33
  ```

---

### **2D Objects**

**Naming Pattern:**
```
[type][2][VarOnZ]In[VarOnY][VarOnX]
```

**Examples:**
- **Histogram of Events vs. Jet Pt and Jet Eta:**
  ```
  h2EventInJetPtJetEta
  ```
- **TProfile of Response vs. Jet Pt and Jet Eta:**
  ```
  p2RespInJetPtJetEta
  ```

**With Additional Cuts:**
- **Include `And` for Multiple Cuts**

**Example for Two Cuts:**
```
h2EventInJetPtJetEtaForJetEta1p33AndZpt50
```

