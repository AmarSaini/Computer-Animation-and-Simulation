using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RunAway : MonoBehaviour {

    public Rigidbody rb;
    private float angle = 0;

    // Use this for initialization
    void Start () {

        rb = GetComponent<Rigidbody>();

        rb.velocity = new Vector3(0, 0, 0);

    }
	
	// Update is called once per frame
	void FixedUpdate() {

        angle += 0.05f;

        rb.velocity = new Vector3(Mathf.Cos(angle), 0, Mathf.Sin(angle));
        rb.velocity *= 10;

    }
}
